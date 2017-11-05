#ifndef MLY_TOKEN_H
#define MLY_TOKEN_H

#include <stdint.h>
#include <stdbool.h>

typedef struct _token {
  uint32_t type;

  uint32_t line_no;
  uint32_t column_no;
  char *line;

  union {
    int64_t number_value;
    bool bool_value;
    float float_value;
    char *string_value;
    char *ident_value;
  };

  struct _token *next;
} token_t;

typedef void (^token_iterate_cb)(token_t *);

token_t *mk_token(uint32_t, uint32_t, uint32_t, char *);

token_t *lex(char *bfr);

void token_iterate(token_t *token_top, token_iterate_cb cb);

void print_tokens(token_t *tokens);

void token_free(token_t *token);

void tokens_free(token_t *tokens);

#endif //MLY_TOKEN_H
