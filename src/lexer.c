#include "lexer.h"
#include "token.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#define TOKEN_BUFFER_SIZE (512)

/**
 * gets the next token and consumses it
 */
struct Token lexer_consume_token(struct Lexer *lexer) {
  
  // the buffer containing the current token buffer
  char buf[TOKEN_BUFFER_SIZE] = {0};
  int  buf_len = 0;
  
  /**
   * classifies a token from a string
   */
  int classify_token(char *buf, int buf_len);

  for (; 
    buf_len == 0 || 
    (
      classify_token(buf, buf_len) != TOKEN_ILLEGAL 
      && *lexer->consume_ptr != 0
    );
    lexer->consume_ptr++
  )
  {
    if (buf_len >= TOKEN_BUFFER_SIZE) {
      /**
       * TODO: register an error
       */
      break;
    }

    // push the current letter to the buffer
    buf[buf_len++] = *lexer->consume_ptr;
  }

  // remove the last letter
  if (buf_len > 0) {
    buf[--buf_len] = 0;
    lexer->consume_ptr--;
  }

  return token_alloc(buf, classify_token(buf, buf_len));
}

/**
 * gets the next token without consuming it
 */
struct Token lexer_peek_token(struct Lexer *lexer) {
  struct Lexer sacrificial_lexer = CREATE_LEXER(lexer->consume_ptr);
  return lexer_consume_token(&sacrificial_lexer);
}

static bool is_identifier(char *buf, int buf_len) {
  if (buf_len == 0) return false;

  if (!isalpha(buf[0]) && buf[0] != '_')
    return false;

  for (; *buf != 0; buf++) {
    if (!isalpha(*buf) && !isdigit(*buf) && buf[0] != '_') {
      return false;
    }
  }

  return true;
}

/**
 * classifies a token from a string
 */
int classify_token(char *buf, int buf_len) {
  if (is_identifier(buf, buf_len)) {
    return TOKEN_IDENTIFIER;
  }

  if (buf_len == 2 && buf[1] == '=') {
    switch (buf[0]) {
      case '=': return TOKEN_EQUALITY;
      case '>': return TOKEN_BIGEQ;
      case '<': return TOKEN_LESSEQ;
      case '+': return TOKEN_ASSIGN_ADD;
      case '-': return TOKEN_ASSIGN_SUB;
      case '*': return TOKEN_ASSIGN_MUL;
      case '/': return TOKEN_ASSIGN_DIV;
      case '&': return TOKEN_ASSIGN_BITAND;
      case '|': return TOKEN_ASSIGN_BITOR;
      case '^': return TOKEN_ASSIGN_BITXOR;
    }
  }

  if (buf_len == 2 && buf[0] == buf[1]) {
    switch (buf[0]) {
      case '+': return TOKEN_INC;
      case '-': return TOKEN_DEC;
    }
  }

  if (buf_len == 1) {
    switch (buf[0]) {
      case '+': return TOKEN_ADD;
      case '-': return TOKEN_SUB;
      case '*': return TOKEN_MUL;
      case '/': return TOKEN_DIV;
      case '&': return TOKEN_BITAND;
      case '|': return TOKEN_BITOR;
      case '^': return TOKEN_BITXOR;
      case '!': return TOKEN_ESCLAM;
    }
  }

  return TOKEN_ILLEGAL;
}
