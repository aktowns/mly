#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <glib.h>
#include <gc.h>

#include "parser.h"
#include "token.h"

void token_iterate(token_t *token_top, token_iterate_cb cb) {
  do {
    cb(token_top);
  } while ((token_top = token_top->next) != NULL);
}

token_t *mk_token(uint32_t type, uint32_t line, uint32_t col, char *line_data) {
  token_t* tok = GC_MALLOC(sizeof(token_t));
  tok->type = type;
  tok->line_no = line;
  tok->column_no = col;
  tok->line = line_data;
  tok->next = NULL;

  return tok;
}

void token_free(token_t *token) {
  switch (token->type) {
    case TOKEN_STRING_LITERAL:
      GC_FREE(token->string_value);
      break;
    case TOKEN_IDENT:
      GC_FREE(token->ident_value);
      break;
    default:
      break;
  }

  GC_FREE(token->line);
  GC_FREE(token);
}

void tokens_free(token_t *tokens) {
  token_t *curr;
  while ((curr = tokens) != NULL) {
    tokens = tokens->next;
    token_free(curr);
  }
}

const char *string_token(token_t *token) {
  switch (token->type) {
    case TOKEN_AST:
      return "*";
    case TOKEN_EQUAL:
      return "=";
    case TOKEN_LET:
      return "let";
    case TOKEN_REC:
      return "rec";
    case TOKEN_IDENT:
      return "ident";
    case TOKEN_IF:
      return "if";
    case TOKEN_INT:
      return "int";
    case TOKEN_THEN:
      return "then";
    case TOKEN_ELSE:
      return "else";
    case TOKEN_LESS_GREATER:
      return "<>";
    case TOKEN_LESS_EQUAL:
      return "<=";
    case TOKEN_GREATER_EQUAL:
      return ">=";
    case TOKEN_LPAREN:
      return "(";
    case TOKEN_RPAREN:
      return ")";
    case TOKEN_MINUS:
      return "-";
    case TOKEN_IN:
      return "in";
    default:
      printf("Unknown token type: %i\n", token->type);
      return "unknown";
  }
}

void print_tokens(token_t *tokens) {
  printf("[ ");
  if (tokens != NULL) {
    token_iterate(tokens, ^(token_t *token) {
      printf("%s(%i:%i) ", string_token(token), token->line_no, token->column_no);
    });
  }
  printf("]\n");
}
