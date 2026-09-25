#ifndef AST_NODE_H
#define AST_NODE_H

#include "token.h"
#include "value.h"

#include <stddef.h>

typedef size_t TokenID;
typedef size_t IdentifierUID;
typedef size_t NodeRef;

enum AstNodeType {
  NODE_ERR,
  NODE_IMM,
  NODE_BINARY_OP,
  NODE_LET_BINDING,
  NODE_CONST_BINDING,
  NODE_STATIC_BINDING,
};

enum ErrorType {
  ERR_MISSING_CLOSE_PARENTHESE,
  ERR_EXPECTED_EXPRESSION,
  ERR_EXPECTED_OP,
  ERR_EXPECTED_BINDING,
  ERR_EXPECTED_IDENTIFIER,
  ERR_EXPECTED_EQ_ASSIGN,
  ERR_EXPECTED_SEMI_COLON,
};

struct ErrNode {
  enum ErrorType type;
  TokenID token; 
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

struct LetBindingNode {
  TokenID id;
  NodeRef expr; 
};

struct ConstBindingNode {
  IdentifierUID id;
  NodeRef expr;
};

struct StaticBindingNode {
  IdentifierUID id;
  NodeRef expr;
};

struct AstNode {
  enum AstNodeType type;
  union {
    struct ErrNode err;
    struct ImmNode imm;
    struct BinOpNode bin_op;
    struct LetBindingNode let_binding;
    struct ConstBindingNode const_binding;
    struct StaticBindingNode static_binding;
  };
};

/**
 * get the string representation of the operation
 */
char *get_bin_op(enum BinOp op);

/**
 * get the string representation of an error
 */
char *get_err_str(enum ErrorType err);

#endif
