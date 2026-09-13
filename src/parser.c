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
struct Parser parse_lexer(struct Lexer lexer) {
  struct Parser parser = { 0 };
  create_parser(&parser);

  /**
   * TODO: parse the code there.
   */
  
  // mark as done
  parser_done(&parser);
  return parser;
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

  return true;
}

/**
 * frees a parser. Rendering it unusable. When freed the done flag is set to true to avoid modify parser in the free state.
 * But create_parser can be used again to make the parser behave like a new fresh one.
 */
void free_parser(struct Parser *parser) {
  if (parser == NULL) return;
  
  parser->capacity   = 0;
  parser->node_count = 0;
  parser->done       = true;

  // free all the nodes
  if (parser->nodes != NULL) {
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
    parser_fix_pointers(parser, &parser->nodes[i]);
  }
}

/**
 * fix the pointers attributes of an ast node.
 * WARNING: do not call that outside of parser_done.
 */
void parser_fix_pointers(struct Parser *parser, struct AstNode *node) {
  if (parser == NULL || node == NULL) return;

  switch (node->type) {
    case NODE_ADD:
      node->add.A += (uintptr_t)parser->nodes;
      node->add.B += (uintptr_t)parser->nodes;
      break;
    case NODE_SUB:
      node->sub.A += (uintptr_t)parser->nodes;
      node->sub.B += (uintptr_t)parser->nodes;
      break;
    case NODE_MUL:
      node->mul.A += (uintptr_t)parser->nodes;
      node->mul.B += (uintptr_t)parser->nodes;
      break;
    case NODE_DIV:
      node->div.A += (uintptr_t)parser->nodes;
      node->div.B += (uintptr_t)parser->nodes;
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
size_t create_syntax_error(struct Parser *parser, struct SynErrNode err) {
  // create a new node
  struct AstNode node             = (struct AstNode) { .type = NODE_SYNTAX_ERR, .err = err };
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
 * creates an add node
 * WARNING: 0 is used as an error sentinel
 */
size_t create_add_node(struct Parser *parser, struct AddNode add) {
  // create a new node
  struct AstNode node             = (struct AstNode) { .type = NODE_ADD, .add = add };
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
 * creates an sub node
 * WARNING: 0 is used as an error sentinel
 */
size_t create_sub_node(struct Parser *parser, struct SubNode sub) {
  // create a new node
  struct AstNode node             = (struct AstNode) { .type = NODE_SUB, .sub = sub };
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
 * creates an mul node
 * WARNING: 0 is used as an error sentinel
 */
size_t create_mul_node(struct Parser *parser, struct MulNode mul) {
  // create a new node
  struct AstNode node             = (struct AstNode) { .type = NODE_SUB, .mul = mul };
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
 * creates an div node
 * WARNING: 0 is used as an error sentinel
 */
size_t create_div_node(struct Parser *parser, struct DivNode div) {
  // create a new node
  struct AstNode node             = (struct AstNode) { .type = NODE_SUB, .div = div };
  size_t         relative_address = parser->node_count; // index

  // push it to the parser
  if (!parser_push_node(parser, &node)) {
    perror("node push failed");
    return 0; // 0 is used as a sentinel because no sane node would point back to the root node.
  }

  // return the relative ptr to the node
  return relative_address;
}
