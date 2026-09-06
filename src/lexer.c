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
    
    // skip blank chars in the front of the the word
    if (is_blank_chr(*lexer->consume_ptr)) {
      if (blank_yet) {
        continue;
      }
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

static bool is_int_literal(char *buf, int buf_len) {
  if (buf_len == 0) return false;

  for (;*buf != 0; buf++) {
    if (!isdigit(*buf)) {
      return false;
    }
  }

  return true;
}

static bool is_float_literal(char *buf, int buf_len) {
  if (buf_len == 0) return false;

  for (; *buf != '.' && *buf != 0; buf++) {
    if (!isdigit(*buf)) {
      return false;
    }
  }
  
  // end of literal
  if (*buf == 0) {
    return true;
  }
  
  // there's a dot, continue
  if (*buf != '.') {
    return false;
  }

  for (; *buf != 0; buf++) {
    if (!isdigit(*buf)) {
      return false;
    }
  }

  return true;
}

static bool is_chr_literal(char *buf, int buf_len) {
  if (buf_len <= 2) return false; // a char literal cannot be empty
  
  // a char literal should start with a single quote
  if (buf[0] != '\'') return false;
  
  // and end with another single quote
  if (buf[buf_len - 1] != '\'') return false;

  return true;
}

static bool is_incomplete_chr_literal(char *buf, int buf_len) {
  // an incomplete char litral should start with a single quote at the start
  if (buf_len == 0 || buf[0] != '\'') return false;

  for (buf++; *buf != 0; buf++) {
    if (*buf == '\'') {
      return false;
    }
  }

  return true;
}

static bool is_str_literal(char *buf, int buf_len) {
  if (buf_len < 2) return false;
  
  // a str literal should start with a double quote
  if (buf[0] != '"') return false;
  
  // and end with another double quote
  if (buf[buf_len - 1] != '"') return false;

  return true;
}

static bool is_incomplete_str_literal(char *buf, int buf_len) {
  // a incomplete str literalral should start with a double quote
  if (buf_len == 0 || buf[0] != '"') return false;

  for (buf++; *buf != 0; buf++) {
    if (*buf == '"') {
      return false;
    }
  }

  return true;
}

#define WORD_TOKEN(word, buf, buf_len, token_type) \
  if (strncmp(buf, word, buf_len) == 0) return token_type;

/**
 * classifies a token from a string
 */
int classify_token(char *buf, int buf_len) {
  // keywords tokens
  WORD_TOKEN("let", buf, buf_len, TOKEN_LET);
  WORD_TOKEN("func", buf, buf_len, TOKEN_FUNC);
  WORD_TOKEN("struct", buf, buf_len, TOKEN_STRUCT);
  WORD_TOKEN("impl", buf, buf_len, TOKEN_IMPL);
  WORD_TOKEN("behavior", buf, buf_len, TOKEN_BEHAVIOR);
  WORD_TOKEN("use", buf, buf_len, TOKEN_USE);
  WORD_TOKEN("static", buf, buf_len, TOKEN_STATIC);
  WORD_TOKEN("const", buf, buf_len, TOKEN_CONST);
  WORD_TOKEN("for", buf, buf_len, TOKEN_FOR);
  WORD_TOKEN("while", buf, buf_len, TOKEN_WHILE);
  WORD_TOKEN("do", buf, buf_len, TOKEN_DO);
  WORD_TOKEN("if", buf, buf_len, TOKEN_IF);
  WORD_TOKEN("else", buf, buf_len, TOKEN_ELSE);

  if (is_identifier(buf, buf_len)) {
    return TOKEN_IDENTIFIER;
  }

  if (is_int_literal(buf, buf_len)) {
    return TOKEN_INT_LITERAL;
  }

  if (is_float_literal(buf, buf_len)) {
    return TOKEN_FLOAT_LITERAL;
  }

  if (is_chr_literal(buf, buf_len)) {
    return TOKEN_CHR_LITERAL;
  }
  else if (is_incomplete_chr_literal(buf, buf_len)) {
    return TOKEN_INCOMPLETE_CHR;
  }

  if (is_str_literal(buf, buf_len)) {
    return TOKEN_STR_LITERAL;
  }
  else if (is_incomplete_str_literal(buf, buf_len)) {
    return TOKEN_INCOMPLETE_STR;
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

  if (buf_len == 3 && buf[2] == '=' && buf[0] == buf[1]) {
    switch (buf[0]) {
      case '<': return TOKEN_ASSIGN_SHL;
      case '>': return TOKEN_ASSIGN_SHR;
    }
  }

  if (buf_len == 2 && buf[0] == buf[1]) {
    switch (buf[0]) {
      case '+': return TOKEN_INC;
      case '-': return TOKEN_DEC;
      case '>': return TOKEN_SHR;
      case '<': return TOKEN_SHL;
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
      case '=': return TOKEN_ASSIGN_EQ;
      case '(': return TOKEN_LPARENTHESES;
      case ')': return TOKEN_RPARENTHESES;
      case ':': return TOKEN_COLON;
      case ',': return TOKEN_COMMA;
      case '.': return TOKEN_DOT;
      case '?': return TOKEN_INTEROG;
      case '[': return TOKEN_LSQRBRACKETS;
      case ']': return TOKEN_RSQRBRACKETS;
      case '{': return TOKEN_LCURLY;
      case '}': return TOKEN_RCURLY;
    }
  }

  return TOKEN_ILLEGAL;
}
