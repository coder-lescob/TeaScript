#ifndef __LEXER_H
#define __LEXER_H

#include "token.h"

#define CREATE_LEXER(STR) \
  (struct Lexer) { .consume_ptr = STR }

struct Lexer {
  char *consume_ptr;
};

/**
 * gets the next token and consumses it
 */
struct Token lexer_consume_token(struct Lexer *lexer);

/**
 * gets the next token without consuming it
 */
struct Token lexer_peek_token(struct Lexer *lexer);

/**
 * classifies a token from a string
 */
int classify_token(char *buf, int buf_len);


#endif
