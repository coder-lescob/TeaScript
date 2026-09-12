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
