#include "token.h"

#include <stdlib.h>
#include <string.h>

/**
 * allocates a token, don't forgot to free it
 */
struct Token token_alloc(char *str, int type) {
  // allocate the string
  size_t len = strlen(str);
  char *hstr = NULL;

  if (str != NULL) {
    hstr = malloc(len + 1);
  }

  if (hstr != NULL) {
    // allocation successful
    // so copy the string
    memcpy(hstr, str, len);
    hstr[len] = 0;
  }

  return (struct Token) {
    .word = hstr,
    // if allocation failed the token ain't valid anymore
    .type = (hstr == NULL)? TOKEN_ILLEGAL : type,
  };
}

/**
 * free a priviously allocated token
 */
void token_free(struct Token *token) {
  if (token == NULL || token->word == NULL) return;
  
  // free the token and set it's type to TOKEN_ILLEGAL
  token->type = TOKEN_ILLEGAL;
  free(token->word);
  token->word = NULL;
}

