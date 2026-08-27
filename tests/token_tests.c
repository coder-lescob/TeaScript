#include <stdio.h>
#include <token.h>
#include <string.h>
#include "test.h"

int test_allocation() {
  struct Token token = token_alloc("this a token...", TOKEN_ILLEGAL);
  
  if (token.word == NULL) return -1;

  token_free(&token);

  return 0;
}

int allocation_fails_returns_illegal_token() {
  struct Token token = token_alloc("this a token...", TOKEN_IDENTIFIER);
  
  if (token.word == NULL) {
    if (token.type != TOKEN_ILLEGAL) {
      return -1;
    }
  }

  token_free(&token);

  return 0;
}

int free_returns_illegal() {
  struct Token token = token_alloc("what", TOKEN_IDENTIFIER);
  token_free(&token);

  if (token.type != TOKEN_ILLEGAL) {
    return -1;
  }

  return 0;
}

int free_sets_word_to_null() {
  struct Token token = token_alloc("interresting", TOKEN_INT_LITERAL);
  token_free(&token);

  if (token.word != NULL) {
    return -1;
  }

  return 0;
}

int token_type_to_str() {
  int success = 1;

  #define test_token_type(type) \
    success &= (strcmp(get_token_type_str(type), #type) == 0)? 1 : 0;
  
  TOKEN_TYPES(test_token_type);

  return success? 0 : -1;

  #undef test_token_type
}

int main(void) {
  TEST(test_allocation);
  TEST(allocation_fails_returns_illegal_token);
  TEST(free_returns_illegal);
  TEST(free_sets_word_to_null);
  TEST(token_type_to_str);

  printf("\n");
  return 0;
}
