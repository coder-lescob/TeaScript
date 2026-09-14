#include "ast_node.h"
#include "token.h"
#include "value.h"

#include <stdio.h>

/**
 * displays a given ast node
 */
void display_ast_node(struct AstNode *node, int level) {
  if (node == NULL) return;

  printf("[%p] ", (void *)node);
  
  for (int i = 1; i < level; i++) {
    printf("|  ");
  }

  if (level > 0) {
    printf("|- ");
  }

  switch (node->type) {
    case NODE_SYNTAX_ERR:
      printf("NODE_SYNTAX_ERR( TOKEN( %s, type = %s ) )\n", node->err.token.word, get_token_type_str(node->err.token.type));
      break;
    case NODE_IMM:
      printf("NODE_IMM( ");
      print_value(&node->imm.imm);
      printf(" )\n");
      break;
    case NODE_BINARY_OP:
      printf("NODE_BINARY_OP ( op = %s, %p, %p )\n", get_bin_op(node->bin_op.op), (void *)node->bin_op.A, (void *)node->bin_op.B);
      display_ast_node(node->bin_op.A, level + 1);
      display_ast_node(node->bin_op.B, level + 1);
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
