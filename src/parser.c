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

/**********************************************************************************************
 *  WARNING: pointers used inside the parser code are relative.                               *
 *  Then they're made absolute when the parser_done is called.                                *
 *  So until the parser is marked done, the pointer fields of                                 *
 *  shall not be thought as valid.                                                            *
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
size_t parse_operand(struct Parser *parser, struct Lexer *lexer) {
  // consume the next token
  struct Token token = lexer_consume_token(lexer);
  
  switch (token.type) {
    // opening parentheses
    case TOKEN_LPARENTHESES:
      // free the open parentheses
      token_free(&token);
    
      // parse inside
      size_t lhs = parse_expression(parser, lexer, 0);
    
      // consume token and if it isn't close parentheses then we're in truble
      token = lexer_consume_token(lexer);
      if (token.type != TOKEN_RPARENTHESES) {
        // oh, oh !
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
size_t parse_expression(struct Parser *parser, struct Lexer *lexer, int binding_power) {
  // parse operand
  size_t lhs = parse_operand(parser, lexer);
  
  while (true) {
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
      return create_syntax_error(parser, (struct ErrNode) { ERR_EXPECTED_OP, op } );
    }

    if (binding_power > power) {
      break;
    }
    
    // consume operator but ignore it since we already know what it's
    {
      struct Token ignore = lexer_consume_token(lexer);
      token_free(&ignore);
    }

    // parse the right hand side
    size_t rhs = parse_expression(parser, lexer, power);
    
    // create the operation
    lhs = create_binary_op_node(parser, (struct BinOpNode) { .op = get_bin_op_for_op(op.type), .A = (struct AstNode *)lhs, .B = (struct AstNode *)rhs } );
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
  // meh, no one care  TODO: make checks
  return type - TOKEN_ADD;
}

/**********************************************************************************************
 *                              node/parser creation methods                                  *
 **********************************************************************************************/

// the entire machinary is based on the idea that I can convert from size_t to uintptr_t and to pointer losslessly
_Static_assert(sizeof(uintptr_t) >= sizeof(size_t), "uintptr_t cannot represent node offsets");

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

  // fix all addresses
  for (size_t i = 0; i < parser->node_count; i++) {
    parser_fix_pointers(&parser->nodes[i], (uintptr_t)parser->nodes);
  }
}

/**
 * fix the pointers attributes of an ast node.
 * adds offset to all the pointers attributes of a node.
 * WARNING: do not call that outside of parser internal functions.
 */
void parser_fix_pointers(struct AstNode *node, uintptr_t offset) {
  if (node == NULL) return;

  switch (node->type) {
    case NODE_BINARY_OP:
      node->bin_op.A = (struct AstNode *)((uintptr_t)node->bin_op.A * sizeof(struct AstNode) + offset);
      node->bin_op.B = (struct AstNode *)((uintptr_t)node->bin_op.B * sizeof(struct AstNode) + offset);
      break;
    default:
      break;
  }
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

/**
 * create a syntax error node
 * WARNING: 0 is used as an error sentinel
 */
size_t create_syntax_error(struct Parser *parser, struct ErrNode err) {
  // create a new node
  struct AstNode node             = (struct AstNode) { .type = NODE_ERR, .err = err };
  size_t         relative_address = parser->node_count; // index

  // push it to the parser
  if (!parser_push_node(parser, &node)) {
    perror("node push failed");
    return 0; // 0 is used as a sentinel because no sane node would point back to the root node.
  }

  // return the relative ptr to the node
  return relative_address;
}

/**
 * creates an immediate node
 *  WARNING: 0 is used as an error sentinel.
 */
size_t create_node_imm(struct Parser *parser, struct ImmNode imm) {
  // create a new node
  struct AstNode node             = (struct AstNode) { .type = NODE_IMM, .imm = imm };
  size_t         relative_address = parser->node_count; // index

  // push it to the parser
  if (!parser_push_node(parser, &node)) {
    perror("node push failed");
    return 0; // 0 is used as a sentinel because no sane node would point back to the root node.
  }

  // return the relative ptr to the node
  return relative_address;
}

/**
 * creates a binary operation node
 * WARNING: 0 is used as an error sentinel
 */
size_t create_binary_op_node(struct Parser *parser, struct BinOpNode op) {
  // create a new node
  struct AstNode node             = (struct AstNode) { .type = NODE_BINARY_OP, .bin_op = op };
  size_t         relative_address = parser->node_count; // index

  // push it to the parser
  if (!parser_push_node(parser, &node)) {
    perror("node push failed");
    return 0; // 0 is used as a sentinel because no sane node would point back to the root node.
  }

  // return the relative ptr to the node
  return relative_address;
}
