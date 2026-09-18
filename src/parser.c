/**********************************************************************************************
 * This file contains all the teascript parser. For teascript I used a pratt parser because   *
 * it is very good at operation priority and that it can treat a lot of things as expression. *
 * Copyright (c) 2026 Gabriel LESCOB. All Rights Reserved.                                    *
 **********************************************************************************************/

#include "parser.h"
#include "token.h"
#include "lexer.h"
#include "value.h"
#include "convert.h"
#include "ast_node.h"

#include <stdbool.h>
#include <errno.h>

/**********************************************************************************************
 *                                      parser code                                           *
 **********************************************************************************************/

/**
 * uses the syntax of teascript to parse a lexer
 * WARNING: parser must be freed after use (use free_parser).
 */
struct Parser parse_lexer(struct Lexer *lexer) {
  struct Parser parser = { 0 };
  create_parser(&parser);
  
  // parse as an expression
  parser.root_node = parse_expression(&parser, lexer, 0);
  
  // mark as done
  parser_done(&parser);
  return parser;
}

/**
 * parses the operand to an expression.
 */
NodeRef parse_operand(struct Parser *parser, struct Lexer *lexer) {
  // consume the next token
  struct Token token = lexer_consume_token(lexer);
  
  switch (token.type) {
    // opening parentheses
    case TOKEN_LPARENTHESES:
      // free the open parentheses
      token_free(&token);
    
      // parse inside
      NodeRef lhs = parse_expression(parser, lexer, 0);
    
      // consume token and if it isn't close parentheses then we're in truble
      token = lexer_consume_token(lexer);
      if (token.type != TOKEN_RPARENTHESES) {
        // oh, oh !
        // now the lhs could leak memory so we need to free it recusivally
        // TODO: fix that issue
        return create_syntax_error(parser, (struct ErrNode) { ERR_MISSING_CLOSE_PARENTHESE, token } );
      }

      // free the close parentheses
      token_free(&token);
      return lhs;
    // a number
    case TOKEN_INT_LITERAL:
    case TOKEN_FLOAT_LITERAL:
      // creates a Value from the literal
      struct Value value = value_from_token_literal(token);
      token_free(&token); // free the token since we don't need it anymore
    
      // create an immediate node
      return create_node_imm(parser, (struct ImmNode) { value } );
    // oopsy
    default:
      return create_syntax_error(parser, (struct ErrNode) { ERR_EXPECTED_EXPRESSION, token } );
  }
}

/**
 * parses an expression
 */
NodeRef parse_expression(struct Parser *parser, struct Lexer *lexer, int binding_power) {
  // parse operand
  NodeRef lhs = parse_operand(parser, lexer);
  
  while (lhs != SIZE_MAX) {
    // get the operator without consuming it
    struct Token op = lexer_peek_token(lexer);
    if (op.type == TOKEN_EOF || op.type == TOKEN_RPARENTHESES) {
      // we're done!
      token_free(&op);
      break;
    }
    
    // get binding power
    int power = get_binding_powers(op.type);
    if (power == -1) { 
      // invalid operator
      // lhs could leak here too
      return create_syntax_error(parser, (struct ErrNode) { ERR_EXPECTED_OP, op } );
    }

    if (binding_power > power) {
      break;
    }
    
    // consume operator but ignore it since we already know what it's
    lexer_ignore_token(lexer);

    // parse the right hand side
    NodeRef rhs = parse_expression(parser, lexer, power);
    
    // create the operation
    lhs = create_binary_op_node(parser, (struct BinOpNode) { .op = get_bin_op_for_op(op.type), .A = lhs, .B = rhs } );
    token_free(&op);
  }
  
  return lhs;
}

/**
 * get the binding power of a token; -1 is returned when that's impossible to get.
 */
int get_binding_powers(enum TokenType type) {
  switch (type) {
    case TOKEN_ADD: return 0;
    case TOKEN_SUB: return 0;
    case TOKEN_MUL: return 1;
    case TOKEN_DIV: return 1;
    default: return -1;
  }
}

/**
 * get the binary operator for any operator token
 */
enum BinOp get_bin_op_for_op(enum TokenType type) {
  // meh, no one cares  TODO: make checks
  return type - TOKEN_ADD;
}

/**********************************************************************************************
 *                              node/parser creation methods                                  *
 **********************************************************************************************/

/**
 * creates a new parser. Initialize it without any node;
 */
bool create_parser(struct Parser *parser) {
  if (parser == NULL || parser->nodes != NULL) {
    errno = EINVAL;
    return false;
  }
  
  // init fields
  parser->nodes      = NULL;
  parser->capacity   = 0;
  parser->node_count = 0;
  parser->done       = false;
  parser->root_node  = 0;

  return true;
}

/**
 * frees a parser. Rendering it unusable. When freed the done flag is set to true to avoid modify parser in the free state.
 * But create_parser can be used again to make the parser behave like a new fresh one.
 */
void free_parser(struct Parser *parser) {
  if (parser == NULL) return;
  
  // cache the node count
  size_t node_count = parser->node_count;
  
  // set capacity and sizes to 0
  parser->capacity   = 0;
  parser->node_count = 0;
  parser->done       = true;
  parser->root_node  = 0;

  // free all the nodes
  if (parser->nodes != NULL) {
    // free all nodes that need to be
    for (size_t i = 0; i < node_count; i++) {
      free_node(&parser->nodes[i]);
    }

    free(parser->nodes);
    parser->nodes = NULL; // avoid dangling ptr
  }
}

/**
 * marks a parser as done such that it's now immutable.
 * obviously it doesn't prevent you to directly access the feilds but any write using parser_push_node 
 * or any other node creation method would result in an error operation not permitted.
 */
void parser_done(struct Parser *parser) {
  if (parser == NULL || parser->nodes == NULL) return;
  
  // freeze the parser
  parser->done = true;
}

/**
 * pushes a node to the parser's nodes
 */
bool parser_push_node(struct Parser *parser, struct AstNode *node) {
  if (parser == NULL || node == NULL) { 
    errno = EINVAL;
    return false; 
  }

  if (parser->done) {
    errno = EPERM;
    return false;
  }

  if (parser->node_count + 1 > parser->capacity || parser->nodes == NULL) {
    // oopsy there isn't enough place allocated...
    size_t new_capacity = parser->capacity * 2 + 1; // add 1 because 2 * 0 == 0 and we really want more place
    struct AstNode *nodes_reallocated = realloc(parser->nodes, new_capacity * sizeof(struct AstNode));

    if (nodes_reallocated == NULL) {
      // oopsy allocation failed, errno set by realloc
      return false;
    }

    // allocation successful
    parser->nodes    = nodes_reallocated;
    parser->capacity = new_capacity;
  }
  
  // push it!
  parser->nodes[parser->node_count++] = *node;

  return true;
}

static NodeRef create_node(struct Parser *parser, struct AstNode *node) {
  NodeRef ref  = parser->node_count; // get index

  // push it to the parser
  if (!parser_push_node(parser, node)) {
    perror("node push failed");
    return SIZE_MAX;
  }

  // return the relative ptr to the node
  return ref;
}

/**
 * create a syntax error node
 * NOTE: SIZE_MAX is used as an error sentinel
 */
NodeRef create_syntax_error(struct Parser *parser, struct ErrNode err) {
  // create a new node
  struct AstNode node = (struct AstNode) { .type = NODE_ERR, .err = err };
  return create_node(parser, &node);
}

/**
 * creates an immediate node
 * NOTE: SIZE_MAX is used as an error sentinel
 */
NodeRef create_node_imm(struct Parser *parser, struct ImmNode imm) {
  // create a new node
  struct AstNode node = (struct AstNode) { .type = NODE_IMM, .imm = imm };
  return create_node(parser, &node); 
}

/**
 * creates a binary operation node
 * NOTE: SIZE_MAX is used as an error sentinel
 */
NodeRef create_binary_op_node(struct Parser *parser, struct BinOpNode op) {
  // create a new node
  struct AstNode node = (struct AstNode) { .type = NODE_BINARY_OP, .bin_op = op };
  return create_node(parser, &node); 
}
