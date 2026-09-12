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
struct Parser parse_lexer(struct Lexer lexer);

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
 * pushes a node to the parser's nodes
 */
bool parser_push_node(struct Parser *parser, struct AstNode *node);

/**
 * create a syntax error node
 * WARNING: 0 is used as an error sentinel
 */
size_t create_syntax_error(struct Parser *parser, struct SynErrNode err);

/**
 * creates an immediate node
 * WARNING: 0 is used as an error sentinel.
 */
size_t create_node_imm(struct Parser *parser, struct ImmNode imm);

/**
 * creates an add node
 * WARNING: 0 is used as an error sentinel
 */
size_t create_add_node(struct Parser *parser, struct AddNode add);

/**
 * creates an sub node
 * WARNING: 0 is used as an error sentinel
 */
size_t create_sub_node(struct Parser *parser, struct SubNode sub);

#endif
