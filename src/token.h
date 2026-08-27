#ifndef __TOKEN_H
#define __TOKEN_H

struct Token {
  char *word;
  int  type;
};

#define TOKEN_TYPES(X) \
  X(TOKEN_ILLEGAL)     \
  X(TOKEN_EOF)         \
                       \
  /**
   * TODO: add keywords tokens   
   */                  \
                       \
  X(TOKEN_IDENTIFIER)  \
                       \
  X(TOKEN_INT_LITERAL) \
  X(TOKEN_FLOAT_LITERAL)\
                       \
  /* operators: math */\
  X(TOKEN_ADD)         \
  X(TOKEN_SUB)         \
  X(TOKEN_MUL)         \
  X(TOKEN_DIV)         \
                       \
  /* operators: logic */\
  X(TOKEN_BITAND)      \
  X(TOKEN_BITOR)       \
  X(TOKEN_BITXOR)      \
  X(TOKEN_ESCLAM)      \
                       \
  /* operators: shifts */ \
  X(TOKEN_SHR)           \
  X(TOKEN_SHL)         \
                       \
  /* operators: comparison */ \
  X(TOKEN_EQUALITY)    \
  X(TOKEN_BIGGER)      \
  X(TOKEN_LESS)        \
  X(TOKEN_BIGEQ)       \
  X(TOKEN_LESSEQ)        \
                       \
  /* assignments */    \
  X(TOKEN_ASSIGN_EQ)   \
  X(TOKEN_ASSIGN_ADD)  \
  X(TOKEN_ASSIGN_SUB)  \
  X(TOKEN_ASSIGN_MUL)  \
  X(TOKEN_ASSIGN_DIV)  \
  X(TOKEN_ASSIGN_BITAND)\
  X(TOKEN_ASSIGN_BITOR)\
  X(TOKEN_ASSIGN_BITXOR)\
  X(TOKEN_ASSIGN_SHR)  \
  X(TOKEN_ASSIGN_SHL)  \
                       \
  X(TOKEN_INC)         \
  X(TOKEN_DEC)         \

#define MAKE_ENUM(name) name,

enum TokenType {
  TOKEN_TYPES(MAKE_ENUM)
};

#undef MAKE_ENUM

/**
 * allocates a token, don't forgot to free it
 */
struct Token token_alloc(char *str, int type);

/**
 * free a priviously allocated token
 */
void token_free(struct Token *token);

/**
 * get the string name for the token type
 */
char *get_token_type_str(enum TokenType type);

#endif
