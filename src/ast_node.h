#ifndef AST_NODE_H
#define AST_NODE_H

#include "token.h"
#include "value.h"

enum AstNodeType {
  NODE_IMM,
  NODE_ADD,
  NODE_SUB,
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

struct AstNode {
  enum AstNodeType type;
  union {
    struct ImmNode imm;
    struct AddNode add;
    struct SubNode sub;
  } value;
};

#endif
