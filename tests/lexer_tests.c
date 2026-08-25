#include <stdio.h>
#include <string.h>
#include <token.h>
#include <lexer.h>
#include <stdint.h>
#include "test.h"

int consume_consums_the_right_len() {
  char *original = "hello, hello, how are you today";
  struct Lexer lexer = CREATE_LEXER(original);
  
  // consume first token shall be hello
  struct Token first_token = lexer_consume_token(&lexer);

  if (lexer.consume_ptr - original != (int16_t)strlen(first_token.word)) {
    printf("expected to consume: %ld consumed: %ld", strlen(first_token.word), lexer.consume_ptr - original);
    printf("original ptr = %s, after consuming = %s\n", original, lexer.consume_ptr);

    token_free(&first_token);
    return -1;
  }

  token_free(&first_token);

  return 0;
}

int peak_does_not_consum() {
  char *original = "hello, hello, how are you today";
  struct Lexer lexer = CREATE_LEXER(original);
  
  // consume first token shall be hello
  struct Token first_token = lexer_peek_token(&lexer);

  if (lexer.consume_ptr - original != 0) {
    printf("expected to consume: 0 consumed: %ld", lexer.consume_ptr - original);
    printf("original ptr = %s, after consuming = %s\n", original, lexer.consume_ptr);

    token_free(&first_token);
    return -1;
  }

  token_free(&first_token);

  return 0;
}

int lexer_tokenizes_identifier() {
  struct Lexer lexer = CREATE_LEXER("use std;");
  struct Token token = lexer_consume_token(&lexer);

  if (strncmp(token.word, "use", strlen(token.word)) != 0) {
    token_free(&token);
    return -1;
  }

  if (token.type != TOKEN_IDENTIFIER) {
    token_free(&token);
    return -1;
  }
  
  token_free(&token);
  return 0;
}

int last_token_is_eof() {
  struct Lexer lexer = CREATE_LEXER("hello!");
  struct Token token;

  int i;
  for (i = 0; i < 50 && (token = lexer_consume_token(&lexer)).type != TOKEN_EOF; i++) {
    token_free(&token);
  }

  if (i == 50) {
    printf("got %s of type: %d\n", token.word, token.type);
    return -1;
  }

  return 0;
}

int ending_with_semi_colon_is_illegal() {
  struct Lexer lexer = CREATE_LEXER("std;\n woaw");

  struct Token token = lexer_consume_token(&lexer);

  if (strcmp(token.word, "std") > 0) {
    token_free(&token);
    return -1;
  }

  token_free(&token);

  return 0;
}

int std_semi_colon_is_illegal() {
  return (classify_token("std;", strlen("std;")) == TOKEN_ILLEGAL)? 0 : -1;
}

int main(void) {
  TEST(consume_consums_the_right_len);
  TEST(peak_does_not_consum);
  TEST(lexer_tokenizes_identifier);
  TEST(last_token_is_eof);
  TEST(ending_with_semi_colon_is_illegal);

  printf("\n");
  return 0;
}
