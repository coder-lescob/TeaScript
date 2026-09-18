/**********************************************************************************************
 * This file contains the teascript parser overview.                                          *
 * For teascript I used a pratt parser because it is very good at operation priority          *
 * and that it can treat a lot of things as expression.                                       *
 * Copyright (c) 2026 Gabriel LESCOB. All Rights Reserved.                                    *
 **********************************************************************************************/

#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "lexer.h"
#include "value.h"
#include "convert.h"
#include "ast_node.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

struct Parser {
  // a dynamic stack to store all the nodes.
  struct AstNode *nodes;
  size_t capacity;
  size_t node_count;
  
  // the index of the root node
  size_t root_node;
  
  // set when parsing is done, prohibates any further actions than reading from the lexer.
  // any manual violation MUST not reallocate the `nodes` field otherwise all the pointers
  // used by the ast nodes would be corrupted.
  bool done;
};

/**********************************************************************************************
 *                                      parser methods                                        *
 **********************************************************************************************/

/**
 * uses the syntax of teascript to parse a lexer
 */
struct Parser parse_lexer(struct Lexer *lexer);

/**
 * parses the operand to an expression.
 */
size_t parse_operand(struct Parser *parser, struct Lexer *lexer);

/**
 * parses an expression
 */
size_t parse_expression(struct Parser *parser, struct Lexer *lexer, int binding_power);

/**
 * get the binding power of a token; -1 is returned when that's impossible to get.
 */
int get_binding_powers(enum TokenType type);

/**
 * get the binary operator for any operator token
 */
enum BinOp get_bin_op_for_op(enum TokenType type);

/**********************************************************************************************
 *                              node/parser creation methods                                  *
 **********************************************************************************************/

/**
 * creates a new parser. Initialize it without any node.
 * WARNING: parser must be freed after use (use free_parser).
 */
bool create_parser(struct Parser *parser);

/**
 * frees a parser. Rendering it unusable. When freed the done flag is set to true to avoid modify parser in the free state.
 * But create_parser can be used again to make the parser behave like a new fresh one.
 */
void free_parser(struct Parser *parser);

/**
 * marks a parser as done such that it's now immutable.
 * obviously it doesn't prevent you to directly access the feilds but any write using parser_push_node 
 * or any other node creation method would result in an error operation not permitted.
 */
void parser_done(struct Parser *parser);

/**
 * fix the pointers attributes of an ast node.
 * adds offset to all the pointers attributes of a node.
 * WARNING: do not call that outside of parser internal functions.
 */
void parser_fix_pointers(struct AstNode *node, uintptr_t offset);

/**
 * pushes a node to the parser's nodes
 */
bool parser_push_node(struct Parser *parser, struct AstNode *node);

/**
 * create a syntax error node
 * WARNING: 0 is used as an error sentinel
 */
size_t create_syntax_error(struct Parser *parser, struct ErrNode err);

/**
 * creates an immediate node
 * WARNING: 0 is used as an error sentinel.
 */
size_t create_node_imm(struct Parser *parser, struct ImmNode imm);

/**
 * creates a binary operation node
 * WARNING: 0 is used as an error sentinel
 */
size_t create_binary_op_node(struct Parser *parser, struct BinOpNode op);

#endif
