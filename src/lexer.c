#include "lexer.h"
#include "token.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#define TOKEN_BUFFER_SIZE (512)

static bool is_blank_chr(char c) {
  return isblank(c) || c == '\n';
}

static bool is_eof_chr(char c) {
  return c == 0;
}

/**
 * gets the next token and consumses it
 */
struct Token lexer_consume_token(struct Lexer *lexer) {
  
  // the buffer containing the current token buffer
  char buf[TOKEN_BUFFER_SIZE] = {0};
  int  buf_len = 0;
  bool blank_yet = true;

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
    
    // the current token is invalid if it has a ' ', '\t' or '\n'
    // tho if it has not started yet, skip theses
    if (is_blank_chr(*lexer->consume_ptr)) {
      if (blank_yet) {
        continue;
      }
      break;
    }

    // push the current letter to the buffer
    buf[buf_len++] = *lexer->consume_ptr;
    blank_yet = false;
  }
  
  // eof file is reached if and only if the consume cursor of the lexer has reached the eof
  bool eof_reached = is_eof_chr(*lexer->consume_ptr);

  // remove the last letter only if the last fetched character made the token illegal.
  if (buf_len > 0 && classify_token(buf, buf_len) == TOKEN_ILLEGAL) {
    buf[--buf_len] = 0;
    lexer->consume_ptr--;
    
    // tho if it's still illegal after that it was probably entirely illegal so keep it as illegal.
    if (classify_token(buf, buf_len) == TOKEN_ILLEGAL) {
      buf[buf_len++] = *(lexer->consume_ptr++);
    }
  }

  if (eof_reached) {
    return (struct Token) { .word = NULL, .type = TOKEN_EOF };
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
      case '>': return TOKEN_BIGGER;
      case '<': return TOKEN_LESS;
      case ';': return TOKEN_SEMI_COLON;
    }
  }

  return TOKEN_ILLEGAL;
}
