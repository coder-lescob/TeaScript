#ifndef AST_NODE_H
#define AST_NODE_H

#include "token.h"
#include "value.h"

enum AstNodeType {
  NODE_SYNTAX_ERR,
  NODE_IMM,
  NODE_ADD,
  NODE_SUB,
  NODE_MUL,
  NODE_DIV,
};

struct SynErrNode {
  struct Token token;
};

struct ImmNode {
  struct Value imm;
};

struct AddNode {
  struct AstNode *A, *B;
};

struct SubNode {
  struct AstNode *A, *B;
};

struct MulNode {
  struct AstNode *A, *B;
};

struct DivNode {
  struct AstNode *A, *B;
};

struct AstNode {
  enum AstNodeType type;
  union {
    struct SynErrNode err;
    struct ImmNode imm;
    struct AddNode add;
    struct SubNode sub;
    struct MulNode mul;
    struct DivNode div;
  };
};

#endif
