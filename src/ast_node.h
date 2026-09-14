#ifndef AST_NODE_H
#define AST_NODE_H

#include "token.h"
#include "value.h"

enum AstNodeType {
  NODE_SYNTAX_ERR,
  NODE_IMM,
  NODE_BINARY_OP,
};

struct SynErrNode {
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
  struct AstNode *A, *B;
};

struct AstNode {
  enum AstNodeType type;
  union {
    struct SynErrNode err;
    struct ImmNode imm;
    struct BinOpNode bin_op;
  };
};

/**
 * displays a given ast node
 */
void display_ast_node(struct AstNode *node, int level);

/**
 * get the string representation of the operation
 */
char *get_bin_op(enum BinOp op);

#endif
