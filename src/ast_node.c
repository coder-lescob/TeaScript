#include "ast_node.h"
#include "token.h"
#include "value.h"

#include <stdio.h>

/**
 * displays a given ast node
 */
void display_ast_node(struct AstNode *nodes_base, struct AstNode *node, int level) {
  if (node == NULL) return;

  for (int i = 1; i < level; i++) {
    printf("|  ");
  }

  if (level > 0) {
    printf("|- ");
  }

  switch (node->type) {
    case NODE_ERR:
      printf("NODE_ERR( err = %s TOKEN( '%s', type = %s ) )\n", get_err_str(node->err.type), node->err.token.word, get_token_type_str(node->err.token.type));
      break;
    case NODE_IMM:
      printf("NODE_IMM( ");
      print_value(&node->imm.imm);
      printf(" )\n");
      break;
    case NODE_BINARY_OP:
      printf("NODE_BINARY_OP ( op = %s )\n", get_bin_op(node->bin_op.op));
      display_ast_node(nodes_base, &nodes_base[node->bin_op.A], level + 1);
      display_ast_node(nodes_base, &nodes_base[node->bin_op.B], level + 1);
      break;
    default: break;
  }
}

/**
 * get the string representation of the operation
 */
char *get_bin_op(enum BinOp op) {
  switch (op) {
    case OP_ADD: return "OP_ADD";
    case OP_SUB: return "OP_SUB";
    case OP_MUL: return "OP_MUL";
    case OP_DIV: return "OP_DIV";
  }
  return NULL;
}

/**
 * get the string representation of an error
 */
char *get_err_str(enum ErrorType err) {
  switch (err) {
    case ERR_MISSING_CLOSE_PARENTHESE: return "ERR_MISSING_CLOSE_PARENTHESE";
    case ERR_EXPECTED_EXPRESSION:      return "ERR_EXPECTED_EXPRESSION";
    case ERR_EXPECTED_OP:              return "ERR_EXPECTED_OP";
    default: return NULL;
  }
}

/**
 * frees an ast node if it needs to be
 */
void free_node(struct AstNode *node) {
  if (node == NULL) return;

  switch (node->type) {
    case NODE_ERR:
      token_free(&node->err.token);
      break;
    default:
      break;
  }
}
