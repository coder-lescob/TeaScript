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
#include <math.h>

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
  parser.root_node = parse_let_binding(&parser, lexer);
  
  // mark as done
  parser_done(&parser);
  return parser;
}

/**
 * parses a binding
 */
NodeRef parse_let_binding(struct Parser *parser, struct Lexer *lexer) {
  if (parser == NULL || lexer == NULL) return SIZE_MAX;
  
  // consume the token
  struct Token token = lexer_consume_token(lexer);
  if (token.type != TOKEN_LET) {
    TokenID tok = push_token(parser, &token);
    return create_syntax_error(parser, (struct ErrNode) { ERR_EXPECTED_BINDING, tok } );
  }
  token_free(&token);
  
  // get the var name
  struct Token name = lexer_consume_token(lexer);
  TokenID nameID    = push_token(parser, &name);

  // verify ID
  if (((struct Token *)parser->token_table.data + nameID)->type != TOKEN_IDENTIFIER) {
    return create_syntax_error(parser, (struct ErrNode) { ERR_EXPECTED_IDENTIFIER, nameID } );
  }

  struct Token eq = lexer_consume_token(lexer);
  if (eq.type != TOKEN_ASSIGN_EQ) {
    TokenID eq_id = push_token(parser, &eq);
    return create_syntax_error(parser, (struct ErrNode) { ERR_EXPECTED_EQ_ASSIGN, eq_id } );
  }
  token_free(&eq);
  
  // parse the expression
  NodeRef expr = parse_expression(parser, lexer, 0);
  
  struct Token semi_colon = lexer_consume_token(lexer);
  if (semi_colon.type != TOKEN_SEMI_COLON) {
    TokenID not_semi_colon = push_token(parser, &semi_colon);
    return create_syntax_error(parser, (struct ErrNode) { ERR_EXPECTED_SEMI_COLON, not_semi_colon });
  }
  token_free(&semi_colon);

  return create_let_binding_node(parser, (struct LetBindingNode) { nameID, expr } );
}

/**
 * parses the operand to an expression.
 */
NodeRef parse_operand(struct Parser *parser, struct Lexer *lexer) {
  if (parser == NULL || lexer == NULL) return SIZE_MAX;

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
        TokenID tok = push_token(parser, &token);
        return create_syntax_error(parser, (struct ErrNode) { ERR_MISSING_CLOSE_PARENTHESE, tok } );
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
      TokenID tok = push_token(parser, &token);
      return create_syntax_error(parser, (struct ErrNode) { ERR_EXPECTED_EXPRESSION, tok } );
  }
}

/**
 * parses an expression
 */
NodeRef parse_expression(struct Parser *parser, struct Lexer *lexer, float binding_power) {
  if (parser == NULL || lexer == NULL) return SIZE_MAX;

  // parse operand
  NodeRef lhs = parse_operand(parser, lexer);
  
  while (lhs != SIZE_MAX) {
    // get the operator without consuming it
    struct Token op = lexer_peek_token(lexer);
    if (op.type == TOKEN_EOF || op.type == TOKEN_RPARENTHESES || op.type == TOKEN_SEMI_COLON) {
      // we're done!
      token_free(&op);
      break;
    }
    
    // get binding power
    BindingPower power = get_binding_powers(op.type);
    if (isnan(power.lhs) || isnan(power.rhs)) {
      // invalid operator
      TokenID tok = push_token(parser, &op);
      return create_syntax_error(parser, (struct ErrNode) { ERR_EXPECTED_OP, tok } );
    }

    if (power.lhs < binding_power) {
      token_free(&op);
      break;
    }
    
    // consume operator but ignore it since we already know what it's
    lexer_ignore_token(lexer);

    // parse the right hand side
    NodeRef rhs = parse_expression(parser, lexer, power.rhs);
    
    // create the operation
    lhs = create_binary_op_node(parser, (struct BinOpNode) { .op = get_bin_op_for_op(op.type), .A = lhs, .B = rhs } );
    token_free(&op);
  }
  
  return lhs;
}

/**
 * get the binding power of a token; 
 * { NAN, NAN } is returned when that's impossible to get aka operator invalid
 */
BindingPower get_binding_powers(enum TokenType type) {
  switch (type) {
    case TOKEN_ADD: 
    case TOKEN_SUB: return (BindingPower) { 1.0f, 1.1f };
    case TOKEN_MUL: 
    case TOKEN_DIV: return (BindingPower) { 2.0f, 2.1f };
    default: return (BindingPower) { NAN, NAN };
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
  if (parser == NULL) {
    errno = EINVAL;
    return false;
  }
  
  // init fields
  parser->nodes       = vec_new(struct AstNode);
  parser->token_table = vec_new(struct Token);
  parser->done        = false;
  parser->root_node   = 0;
  parser->err         = false;

  return true;
}

/**
 * frees a parser. Rendering it unusable. When freed the done flag is set to true to avoid modify parser in the free state.
 * But create_parser can be used again to make the parser behave like a new fresh one.
 */
void free_parser(struct Parser *parser) {
  if (parser == NULL) return;
  
  // set capacity and sizes to 0
  parser->done       = true;
  parser->root_node  = 0;

  // free all the nodes
  vec_free(&parser->nodes);
  
  // free all tokens
  if (parser->token_table.data != NULL) {
    for (size_t i = 0; i < parser->token_table.count; i++) {
      token_free((struct Token *)parser->token_table.data + i);
    }
  }

  vec_free(&parser->token_table);
}

/**
 * marks a parser as done such that it's now immutable.
 * obviously it doesn't prevent you to directly access the feilds but any write using parser_push_node 
 * or any other node creation method would result in an error operation not permitted.
 */
void parser_done(struct Parser *parser) {
  if (parser == NULL) return;
  
  // freeze the parser
  parser->done = true;
}

/**
 * pushes a node to the parser's nodes
 */
bool parser_push_node(struct Parser *parser, struct AstNode *node) {
  return vec_push(&parser->nodes, (void *)node) == 0;
}

static NodeRef create_node(struct Parser *parser, struct AstNode *node) {
  NodeRef ref  = parser->nodes.count; // get index

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
  // set the err flag to true;
  parser->err = true;

  // push the node
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

/**
 * creates a let binding node
 * NOTE: SIZE_MAX is used as an error sentinel
 */
NodeRef create_let_binding_node(struct Parser *parser, struct LetBindingNode let) {
  struct AstNode node = { .type = NODE_LET_BINDING, .let_binding = let };
  return create_node(parser, &node);
}

/**
 * pushes a token to the token table and return it's index
 * NOTE: if it fails to push the token gets freed, but 
 * if it succeed the ownership of the token is for the token_table thus token is set to { NULL, TOKEN_ILLEGAL }
 */
TokenID push_token(struct Parser *parser, struct Token *token) {
  if (parser == NULL || token == NULL) {
    errno = EINVAL;
    token_free(token);
    return SIZE_MAX; 
  }

   if (parser->done) {
    errno = EPERM;
    token_free(token);
    return SIZE_MAX;
  }
  
  // get the index of the new token
  size_t idx = parser->token_table.count;
  
  // push!
  if (vec_push(&parser->token_table, token) != 0) {
    token_free(token);
    return SIZE_MAX;
  }
  
  // avoid dangling
  token->word = NULL;
  token->type = TOKEN_ILLEGAL;

  return idx;
}

/**
 * displays a given ast node
 */
void display_ast(struct Parser *parser, struct AstNode *node, int level) {
  if (node == NULL || parser == NULL) return;

  for (int i = 1; i < level; i++) {
    printf("|  ");
  }

  if (level > 0) {
    printf("|- ");
  }

  switch (node->type) {
    case NODE_ERR:
      printf("NODE_ERR( err = %s TOKEN( '%s', type = %s ) )\n", 
             get_err_str(node->err.type), 
             ((struct Token *)parser->token_table.data + node->err.token)->word,
             get_token_type_str(((struct Token *)parser->token_table.data + node->err.token)->type)
      );
      break;
    case NODE_IMM:
      printf("NODE_IMM( ");
      print_value(&node->imm.imm);
      printf(" )\n");
      break;
    case NODE_BINARY_OP:
      printf("NODE_BINARY_OP ( op = %s )\n", get_bin_op(node->bin_op.op));
      display_ast(parser, (struct AstNode *)parser->nodes.data + node->bin_op.A, level + 1);
      display_ast(parser, (struct AstNode *)parser->nodes.data + node->bin_op.B, level + 1);
      break;
    case NODE_LET_BINDING:
      printf("NODE_LET_BINDING ( '%s' )\n", ((struct Token *)parser->token_table.data + node->let_binding.id)->word);
      display_ast(parser, (struct AstNode *)parser->nodes.data + node->let_binding.expr, level + 1);
      break;
    default:
      printf("UNKNOWN_NODE ( %d )\n", node->type);
      break;
  }
}
