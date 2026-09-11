#ifndef VALUE_H
#define VALUE_H

#include <stdint.h>
#include <stddef.h>

#include "token.h"

enum ValueType {
  VALUE_INT,
  VALUE_FLOAT,
  VALUE_STR,
  VALUE_CHR,
  VALUE_STRUCT,
};

struct String {
  char *text;
  size_t len;
};

struct Struct {
  struct Value *members;
  size_t num_members;
};

struct Value {
  enum ValueType type;
  union {
    uint64_t      int_value;
    double        float_value;
    struct String str;
    char          chr;
    struct Struct struct_value;
  } value;
};

/**
 * converts a token to a value.
 */
struct Value value_from_token_literal(struct Token token);

#endif
