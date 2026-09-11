#include "value.h"
#include "token.h"
#include "convert.h"

/**
 * converts a token to a value.
 */
struct Value value_from_token_literal(struct Token token) {
  switch (token.type) {
    case TOKEN_INT_LITERAL:
      return (struct Value) { .type = VALUE_INT, .value = { .int_value = { convert_str_to_int(token.word) } } };

    case TOKEN_FLOAT_LITERAL:
      return (struct Value) { .type = VALUE_FLOAT, .value = { .float_value = { convert_str_to_float(token.word) } } };

    case TOKEN_CHR_LITERAL:
      return (struct Value) { .type = VALUE_CHR, .value = { .chr_value = { token.word[0] } } };
  }
}
