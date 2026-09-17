#include "value.h"
#include "token.h"
#include "convert.h"

/**
 * converts a token to a value.
 */
struct Value value_from_token_literal(struct Token token) {
  switch (token.type) {
    case TOKEN_INT_LITERAL:
      return (struct Value) { .type = VALUE_INT, .int_value = convert_raw_str_to_int(token.word) }; // trust the lexer

    case TOKEN_FLOAT_LITERAL:
      return (struct Value) { .type = VALUE_FLOAT, .float_value = convert_raw_str_to_float(token.word) }; // trust the lexer

    case TOKEN_CHR_LITERAL:
      return (struct Value) { .type = VALUE_CHR, .chr_value = token.word[0] };
  }
}

/**
 * displays a value at the screen
 */
void print_value(struct Value *value) {
  if (value == NULL) return;

  switch (value->type) {
    case VALUE_INT:
      printf("VALUE( %ld )", value->int_value);
      break;

    case VALUE_FLOAT:
      printf("VALUE( %f )", value->float_value);
      break;

    case VALUE_CHR:
      printf("VALUE( '%c' )", value->chr_value);
      break;

    /**
     * TODO: add the other values
     */

    default:
      printf("VALUE( NOT IMPLEMENTED )");
      break;
  }
}
