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
#include <string.h>

VEC_IMPL(AstNode)
VEC_IMPL(Token)
VEC_IMPL(NodeRef)
VEC_IMPL(Vec(NodeRef))

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
  parser.root_node = parse_scope(&parser, lexer);
  
  // mark as done
  parser_done(&parser);
  return parser;
}

/**
 * parses a scope
 */
NodeRef parse_scope(struct Parser *parser, struct Lexer *lexer) {
  if (parser == NULL || lexer == NULL) return SIZE_MAX;
  
  NodeRef statement;
  struct ScopeNode scope = { .scope = create_scope(parser) };

  while (1) {
    // reset the statement
    statement = SIZE_MAX;

    // get the token to choose what to call
    struct Token token = lexer_peek_token(lexer);
    
    if (token.type == TOKEN_EOF) {
      break;
    }
    else if (token.type == TOKEN_LET) {
      statement = parse_let_binding(parser, lexer);
    }
    else if (token.type == TOKEN_LCURLY) {
      lexer_ignore_token(lexer);
      statement = parse_scope(parser, lexer);
    }
    else if (token.type == TOKEN_RCURLY) {
      token_free(&token);
      lexer_ignore_token(lexer);
      break;
    }
    else {
      TokenID tok = push_token(parser, &token);
      statement = create_syntax_error(parser, (struct ErrNode) { .type = ERR_EXPECTED_STATEMENT, .token = tok });
    }

    token_free(&token);
    push_to_scope(parser, &scope, statement);
  }

  return create_scope_node(parser, scope);
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
  parser->nodes       = vec_new(AstNode);
  parser->token_table = vec_new(Token);
  parser->scopes_table= vec_new(Vec(NodeRef));
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
  vec_free(parser->nodes);
  
  // free all tokens
  if (parser->token_table.data != NULL) {
    for (size_t i = 0; i < parser->token_table.count; i++) {
      token_free((struct Token *)parser->token_table.data + i);
    }
  }

  vec_free(parser->token_table);
  
  if (parser->scopes_table.data != NULL) {
    for (size_t i = 0; i < parser->scopes_table.count; i++) {
      vec_free(parser->scopes_table.data[i]);
    }
  }

  vec_free(parser->scopes_table);
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
  if (parser == NULL || node == NULL) return -1;
  return VEC_METHOD(push,AstNode)(&parser->nodes, *node) == 0;
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
 * get the scope index of now.
 */
ScopeRef create_scope(struct Parser *parser) {
  ScopeRef ref = parser->scopes_table.count;
  
  if (VEC_METHOD(push,Vec(NodeRef))(&parser->scopes_table, vec_new(NodeRef)) != 0) {
    return SIZE_MAX;
  }

  return ref;
}

/**
 * adds a node to the scopes_table
 */
int push_to_scope(struct Parser *parser, struct ScopeNode *node, NodeRef ref) {
  
  // get a copy of the scope_node
  // NOTE: here we don't own the scope and it's why we don't free it
  Vec(NodeRef) node_scope;
  if (VEC_METHOD(get_item,Vec(NodeRef))(&parser->scopes_table, node->scope, &node_scope) != 0) {
    return -1;
  }

  if (VEC_METHOD(push,NodeRef)(&node_scope, ref) != 0) {
    return -1;
  }

  return VEC_METHOD(set_item,Vec(NodeRef))(&parser->scopes_table, node->scope, node_scope);
}

/**
 * creates a scope node
 * NOTE: SIZE_MAX is used as an error sentinel
 */
NodeRef create_scope_node(struct Parser *parser, struct ScopeNode scope) {
  struct AstNode node = { .type = NODE_SCOPE, .scope = scope };
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
  if (VEC_METHOD(push,Token)(&parser->token_table, *token) != 0) {
    token_free(token);
    return SIZE_MAX;
  }
  
  // avoid dangling
  token->word = NULL;
  token->type = TOKEN_ILLEGAL;

  return idx;
}

static void display_level(int level) {
  for (int i = 1; i < level; i++) {
    printf("│  ");
  }

  if (level > 0) {
    printf("├─ ");
  }
}

/**
 * displays a given ast node
 */
void display_ast(struct Parser *parser, struct AstNode *node, int level) {
  if (node == NULL || parser == NULL) return;
  
  display_level(level);

  switch (node->type) {
    case NODE_ERR:
      Token err;
      if (VEC_METHOD(get_item, Token)(&parser->token_table, node->err.token, &err) != 0) {
        printf("Node::Err( %s TOKEN ( unable to get token: %s ) )\n", get_err_str(node->err.type), strerror(errno));
        break;
      }

      printf("Node::Err( err = %s TOKEN( '%s', type = %s ) )\n", 
             get_err_str(node->err.type), 
             err.word,
             get_token_type_str(err.type)
      );
      break;
    case NODE_IMM:
      printf("Node::Imm( ");
      print_value(&node->imm.imm);
      printf(" )\n");
      break;
    case NODE_BINARY_OP:
      printf("Node::BinOp ( op: %s )\n", get_bin_op(node->bin_op.op));

      AstNode lhs, rhs;
      if (node->bin_op.A != SIZE_MAX && VEC_METHOD(get_item, AstNode)(&parser->nodes, node->bin_op.A, &lhs) == 0) {
        display_ast(parser, &lhs, level + 1);
      }
      else {
        display_level(level + 1);
        perror("Unable to get lhs node");
      }

      if (node->bin_op.B != SIZE_MAX && VEC_METHOD(get_item, AstNode)(&parser->nodes, node->bin_op.B, &rhs) == 0) {
        display_ast(parser, &rhs, level + 1);
      }
      else {
        display_level(level + 1);
        perror("Unable to get rhs node");
      }

      break;
    case NODE_LET_BINDING:

      Token name;
      if (node->let_binding.id != SIZE_MAX && VEC_METHOD(get_item, Token)(&parser->token_table, node->let_binding.id, &name) != 0) {
        printf("Node::LetBinding ( Unable to fetch varable name )\n");
        break;
      }

      printf("Node::LetBinding ( var_name: '%s' )\n", name.word);

      AstNode expr;
      if (node->let_binding.expr != SIZE_MAX && VEC_METHOD(get_item,AstNode)(&parser->nodes, node->let_binding.expr, &expr) == 0) {
        display_ast(parser, &expr, level + 1);
      }
      else {
        display_level(level + 1);
        perror("Unable to get expression");
      }
      break;
    case NODE_SCOPE:
      printf("Node::Scope\n");
      
      Vec(NodeRef) scope;
      if (node->scope.scope != SIZE_MAX && VEC_METHOD(get_item,Vec(NodeRef))(&parser->scopes_table, node->scope.scope, &scope) != 0) {
        display_level(level + 1);
        perror("Unable to get scope");
        break;
      }

      NodeRef ref;
      struct AstNode child_node;
      for (size_t i = 0; i < scope.count; i++) {
        if (VEC_METHOD(get_item,NodeRef)(&scope, i, &ref) == 0 && ref != SIZE_MAX
         && VEC_METHOD(get_item,AstNode)(&parser->nodes, ref, &child_node) == 0) {
          display_ast(parser, &child_node, level + 1);
        }
        else {
          display_level(level + 1);
          perror("Failed to get node inside of scope");
        }
      }
      break;
    default:
      printf("Node::Unknown ( %d )\n", node->type);
      break;
  }
}
