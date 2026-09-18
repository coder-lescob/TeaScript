#ifndef AST_NODE_H
#define AST_NODE_H

#include "token.h"
#include "value.h"

#include <stddef.h>

typedef size_t NodeRef;

enum AstNodeType {
  NODE_ERR,
  NODE_IMM,
  NODE_BINARY_OP,
};

enum ErrorType {
  ERR_MISSING_CLOSE_PARENTHESE,
  ERR_EXPECTED_EXPRESSION,
  ERR_EXPECTED_OP,
};

struct ErrNode {
  enum ErrorType type;
  struct Token token;
};

struct ImmNode {
  struct Value imm;
};

enum BinOp {
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
};

struct BinOpNode {
  enum BinOp op;
  NodeRef A, B;
};

struct AstNode {
  enum AstNodeType type;
  union {
    struct ErrNode err;
    struct ImmNode imm;
    struct BinOpNode bin_op;
  };
};

/**
 * displays a given ast node
 */
void display_ast_node(struct AstNode *nodes_base, struct AstNode *node, int level);

/**
 * get the string representation of the operation
 */
char *get_bin_op(enum BinOp op);

/**
 * get the string representation of an error
 */
char *get_err_str(enum ErrorType err);

/**
 * frees an ast node if it needs to be
 */
void free_node(struct AstNode *node);

#endif
