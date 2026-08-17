#ifndef __TOKEN_H
#define __TOKEN_H

struct Token {
  char *word;
  int  type;
};

enum {
  TOKEN_ILLEGAL,
  TOKEN_EOF,

  /**
   * TODO: add keywords tokens
   */

  TOKEN_INT_LITERAL,
  TOKEN_FLOAT_LITERAL,

  // operators: math
  TOKEN_ADD,
  TOKEN_SUB,
  TOKEN_MUL,
  TOKEN_DIV,

  // operators: logic
  TOKEN_BITAND,
  TOKEN_BITOR,
  TOKEN_BITXOR,
  TOKEN_ESCLAM,

  // operators: shifts
  TOKEN_SHR,
  TOKEN_SHL,

  // operators: comparison
  TOKEN_EQUALITY,
  TOKEN_BIGGER,
  TOKEN_LESS,
  TOKEN_BIGEQ,
  TOKEN_LESSEQ,

};

/**
 * allocates a token, don't forgot to free it
 */
struct Token token_alloc(char *str, int type);

/**
 * free a priviously allocated token
 */
void token_free(struct Token *token);

#endif
