#include "ast_node.h"
#include "token.h"
#include "value.h"

#include <stdio.h>

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
    case ERR_EXPECTED_BINDING:         return "ERR_EXPECTED_BINDING";
    case ERR_EXPECTED_IDENTIFIER:      return "ERR_EXPECTED_IDENTIFIER";
    case ERR_EXPECTED_EQ_ASSIGN:       return "ERR_EXPECTED_EQ_ASSIGN";
    case ERR_EXPECTED_SEMI_COLON:      return "ERR_EXPECTED_SEMI_COLON";
    default: return NULL;
  }
}
