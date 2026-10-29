#ifndef AST_NODE_H
#define AST_NODE_H

#include "token.h"
#include "value.h"

#include <stddef.h>

typedef size_t TokenID;
typedef size_t IdentifierUID;
typedef size_t NodeRef;
typedef size_t ScopeRef;

enum AstNodeType {
  NODE_ERR,
  NODE_IMM,
  NODE_IDENTIFIER,
  NODE_BINARY_OP,
  NODE_BINDING,
  NODE_SCOPE,
  NODE_IF,
};

enum ErrorType {
  ERR_MISSING_CLOSE_PARENTHESE,
  ERR_EXPECTED_EXPRESSION,
  ERR_EXPECTED_OP,
  ERR_EXPECTED_BINDING,
  ERR_EXPECTED_IDENTIFIER,
  ERR_EXPECTED_EQ_ASSIGN,
  ERR_EXPECTED_SEMI_COLON,
  ERR_EXPECTED_STATEMENT,
  ERR_EXPECTED_IF,
  ERR_EXPECTED_LCURLY,
  ERR_EXPECTED_RCURLY,
};

struct ErrNode {
  enum ErrorType type;
  TokenID token; 
};

struct ImmNode {
  struct Value imm;
};

struct IdentifierNode {
  TokenID token;
};

enum BinOp {
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_EQ,
};

struct BinOpNode {
  enum BinOp op;
  NodeRef A, B;
};

enum BindingKind {
  BINDING_LET,
  BINDING_CONST,
  BINDING_STATIC,
};

struct BindingNode {
  enum BindingKind kind;
  TokenID id;
  NodeRef expr; 
};

struct ScopeNode {
  ScopeRef scope;
};

struct IfStatementNode {
  NodeRef ifscope;
  NodeRef cond;
  // If else branch is SIZE_MAX, then there are not else
  // else if are representated as this elsebranch pointing towards an if node
  NodeRef elsebranch;
};

struct AstNode {
  enum AstNodeType type;
  union {
    struct ErrNode err;
    struct ImmNode imm;
    struct IdentifierNode identifier;
    struct BinOpNode bin_op;
    struct BindingNode binding;
    struct ScopeNode scope;
    struct IfStatementNode ifnode;
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

/**
 * get the binding kind as a string
 */
char *get_binding_str(enum BindingKind bind);

#endif
