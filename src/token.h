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
  /* literals */       \
  X(TOKEN_INT_LITERAL) \
  X(TOKEN_FLOAT_LITERAL)\
  X(TOKEN_INCOMPLETE_FLOAT)\
  X(TOKEN_CHR_LITERAL) \
  X(TOKEN_STR_LITERAL) \
  X(TOKEN_INCOMPLETE_CHR)\
  X(TOKEN_INCOMPLETE_STR)\
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
                       \
  /* symboels */       \
  X(TOKEN_SEMI_COLON)  \
  X(TOKEN_COLON)       \
  X(TOKEN_COMMA)       \
  X(TOKEN_DOT)         \
  X(TOKEN_INTEROG)     \
  X(TOKEN_RPARENTHESES)\
  X(TOKEN_LPARENTHESES)\
  X(TOKEN_LSQRBRACKETS)\
  X(TOKEN_RSQRBRACKETS)\
  X(TOKEN_LCURLY)      \
  X(TOKEN_RCURLY)      \
                       \
  /* keywords */       \
  X(TOKEN_LET)         \
  X(TOKEN_FUNC)        \
  X(TOKEN_STRUCT)      \
  X(TOKEN_IMPL)        \
  X(TOKEN_BEHAVIOR)    \
  X(TOKEN_USE)         \
  X(TOKEN_STATIC)      \
  X(TOKEN_CONST)       \
  X(TOKEN_FOR)         \
  X(TOKEN_WHILE)       \
  X(TOKEN_DO)          \
  X(TOKEN_IF)          \
  X(TOKEN_ELSE)        \

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
