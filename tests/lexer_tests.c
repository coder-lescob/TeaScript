#include <stdio.h>
#include <string.h>
#include <token.h>
#include <lexer.h>
#include <stdint.h>

#include "test.h"

int consume_consums_the_right_len(void) {
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

int peek_does_not_consume(void) {
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

int last_token_is_eof(void) {
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

int ending_with_semi_colon_is_illegal(void) {
  struct Lexer lexer = CREATE_LEXER("std;\n woaw");

  struct Token token = lexer_consume_token(&lexer);

  if (strcmp(token.word, "std") > 0) {
    token_free(&token);
    return -1;
  }

  token_free(&token);

  return 0;
}

int std_semi_colon_is_illegal(void) {
  return (classify_token("std;", strlen("std;")) == TOKEN_ILLEGAL)? 0 : -1;
}

/**
 * @warning: this test has been written by AI because writing test code is boring
 */
int test_lexer_consume_all_tokens(void)
{
    /*
     * Each entry contains:
     *   - source text that should produce exactly one token
     *   - expected token type
     *
     * A fresh lexer is used for every test so that each token is tested
     * independently and tokens which consume multiple characters cannot
     * interfere with subsequent tests.
     */
  static const struct {
    const char *source;
    int expected;
  } tests[] = {
      { "§",          TOKEN_ILLEGAL },

      { "o1",         TOKEN_IDENTIFIER },
      { "ezbdez",     TOKEN_IDENTIFIER },
      { "cdcb_0254u", TOKEN_IDENTIFIER },

      { "123",        TOKEN_INT_LITERAL },
      { "123.456",    TOKEN_FLOAT_LITERAL },
      { "'a'",        TOKEN_CHR_LITERAL },
      { "\"hello\"",  TOKEN_STR_LITERAL },

      { "+",           TOKEN_ADD },
      { "-",           TOKEN_SUB },
      { "*",           TOKEN_MUL },
      { "/",           TOKEN_DIV },

      { "&",           TOKEN_BITAND },
      { "|",           TOKEN_BITOR },
      { "^",           TOKEN_BITXOR },
      { "!",           TOKEN_ESCLAM },

      { ">>",          TOKEN_SHR },
      { "<<",          TOKEN_SHL },

      { "==",          TOKEN_EQUALITY },
      { ">",           TOKEN_BIGGER },
      { "<",           TOKEN_LESS },
      { ">=",          TOKEN_BIGEQ },
      { "<=",          TOKEN_LESSEQ },

      { "=",           TOKEN_ASSIGN_EQ },
      { "+=",          TOKEN_ASSIGN_ADD },
      { "-=",          TOKEN_ASSIGN_SUB },
      { "*=",          TOKEN_ASSIGN_MUL },
      { "/=",          TOKEN_ASSIGN_DIV },
      { "&=",          TOKEN_ASSIGN_BITAND },
      { "|=",          TOKEN_ASSIGN_BITOR },
      { "^=",          TOKEN_ASSIGN_BITXOR },
      { ">>=",         TOKEN_ASSIGN_SHR },
      { "<<=",         TOKEN_ASSIGN_SHL },

      { "++",          TOKEN_INC },
      { "--",          TOKEN_DEC },

      { ";",           TOKEN_SEMI_COLON },
      { ":",           TOKEN_COLON },
      { ",",           TOKEN_COMMA },
      { ".",           TOKEN_DOT },
      { "?",           TOKEN_INTEROG },

      { ")",           TOKEN_RPARENTHESES },
      { "(",           TOKEN_LPARENTHESES },
      { "[",           TOKEN_LSQRBRACKETS },
      { "]",           TOKEN_RSQRBRACKETS },
      { "{",           TOKEN_LCURLY },
      { "}",           TOKEN_RCURLY },

      { "let",         TOKEN_LET },
      { "func",        TOKEN_FUNC },
      { "struct",      TOKEN_STRUCT },
      { "impl",        TOKEN_IMPL },
      { "behavior",    TOKEN_BEHAVIOR },
      { "use",         TOKEN_USE },
      { "static",      TOKEN_STATIC },
      { "const",       TOKEN_CONST },
      { "for",         TOKEN_FOR },
      { "while",       TOKEN_WHILE },
      { "do",          TOKEN_DO },
      { "if",           TOKEN_IF },
      { "else",        TOKEN_ELSE },
  };
    
  // get the number of tokens in the tests
  const size_t test_count = sizeof(tests) / sizeof(tests[0]);
  size_t failed = 0;
    
  // loop over all tests tokens
  for (size_t i = 0; i < test_count; i++) {
    // create a lexer containing the token and consume it
    struct Lexer lexer = CREATE_LEXER((char *)tests[i].source);
    struct Token actual = lexer_consume_token(&lexer);
  
    // verify result
    if (actual.type != tests[i].expected) {
      fprintf(
        stderr,
        "[FAIL] test #%zu: input \"%s\": "
        "expected %s, got %s %s\n",
        i,
        tests[i].source,
        get_token_type_str(tests[i].expected),
        actual.word,
        get_token_type_str(actual.type)
      );

      token_free(&actual);
      failed++;
    }
    else {
      printf(
        "[PASS] test #%zu: \"%s\" -> %s\n",
        i,
        tests[i].source,
        get_token_type_str(actual.type)
      );
    }

    token_free(&actual);
  }

  /*
  * EOF is special: it is not represented by source text, so test it
  * separately by consuming from an empty lexer.
  */
  {
    struct Lexer lexer = CREATE_LEXER("");
    struct Token actual = lexer_consume_token(&lexer);

    if (actual.type != TOKEN_EOF) {
      fprintf(
         stderr,
         "[FAIL] EOF test: expected %s, got %s\n",
         get_token_type_str(TOKEN_EOF),
         get_token_type_str(actual.type)
      );

      token_free(&actual);
      failed++;
    }
    else {
      printf(
        "[PASS] EOF test: -> %s\n",
        get_token_type_str(actual.type)
      );
    }

    token_free(&actual);
  }
  
  if (failed > 0) {
    fprintf(stderr, "[FAIL] lexer_consume_token: %zu/%zu public token types failed\n", failed, test_count + 1);
    return -1;
  }

  printf(
    "[PASS] lexer_consume_token: all %zu public token types passed\n",
    test_count + 1
  );

  return 0;
}

int main(void) {
  TEST(consume_consums_the_right_len);
  TEST(peek_does_not_consume);
  TEST(last_token_is_eof);
  TEST(ending_with_semi_colon_is_illegal);
  TEST(test_lexer_consume_all_tokens);

  printf("\n");
  return 0;
}
