#ifndef MLY_AST_H
#define MLY_AST_H

#include <glib.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "env.h"
#include "token.h"

typedef enum {
  EQUALITY_EQUAL,
  EQUALITY_NOT_EQUAL,
  RELATIONAL_LESS,
  RELATIONAL_LESS_EQUAL,
  RELATIONAL_GREATER,
  RELATIONAL_GREATER_EQUAL,
  ADDITIVE_ADD,
  ADDITIVE_SUB,
  MULTIPLICATIVE_MUL,
  MULTIPLICATIVE_DIV,
  MULTIPLICATIVE_MOD,
  LOGICAL_AND,
  LOGICAL_OR
} binary_op_t;

typedef enum {
  AST_UNIT,
  AST_BOOL,
  AST_LET,
  AST_LET_REC,
  AST_VAR_REF,
  AST_BINARY_OP,
  AST_INT,
  AST_STRING,
  AST_IF,
  AST_TYPEDECL,
  AST_TYPED,
  AST_APPLY,
  AST_TUPLE_TYPE,
  AST_TUPLE
} ast_type_t;

typedef struct {
  uint32_t line_no;
  uint32_t column_no;
  char *line;
} location_t;

typedef struct {
  char *display_name;
  char *name;

  location_t location;
} symbol_t;

typedef struct param {
  symbol_t *ident;
  struct ast *type;

  location_t location;
} param_t;

typedef struct {
  symbol_t *symbol;
  GList *params;
  struct ast *body;
  struct ast *ret_type;

  location_t location;
} func_def_t;

#define CHECK_AST_TYPE(x, type) g_assert((x)->ast_type == (type))

#define LOC(x)        (x)->location

#define LET(x)        (x)->data.ast_let
#define LET_REC(x)    (x)->data.ast_let_rec
#define VAR_REF(x)    (x)->data.ast_var_ref
#define IF(x)         (x)->data.ast_if
#define TYPE_DECL(x)  (x)->data.ast_type_decl
#define TYPED(x)      (x)->data.ast_typed
#define APPLY(x)      (x)->data.ast_apply
#define TUPLE_TYPE(x) (x)->data.ast_tuple_type
#define TUPLE(x)      (x)->data.ast_tuple
#define BINARY_OP(x)  (x)->data.ast_binary_op
#define INT(x)        (x)->data.ast_int
#define STR(x)        (x)->data.ast_string
#define BOOL(x)       (x)->data.ast_bool

typedef struct ast {
  ast_type_t ast_type;

  location_t location;

  union {
    struct {
      symbol_t *symbol;
      struct ast *bound;
      struct ast *body;
      struct ast *type;
    } ast_let;
    struct {
      func_def_t *func;
      struct ast *body;
    } ast_let_rec;
    struct {
      struct ast *left;
      struct ast *right;
      binary_op_t op;
    } ast_binary_op;
    struct {
      struct ast *expr;
      struct ast *true_body;
      struct ast *false_body;
    } ast_if;
    struct {
      char *name;
      symbol_t *symbol;
      struct ast *expr;
    } ast_type_decl;
    struct {
      struct ast *child;
      struct ast *type;
    } ast_typed;
    struct {
      struct ast *callee;
      GList *args;
    } ast_apply;
    struct {
      symbol_t *symbol;
    } ast_var_ref;
    struct {
      GList *elem_types;
    } ast_tuple_type;
    struct {
      GList *elems;
    } ast_tuple;
    int32_t ast_int;
    bool ast_bool;
    char *ast_string;
  } data;
} ast_t;

typedef void (^ast_iterate_cb)(ast_t *);

location_t loc_from_token(token_t *token);

char *pp_ast(GList *);

ast_t *mk_ast(ast_type_t type);

ast_t *ast_unit();

ast_t *ast_bool(bool val);

ast_t *ast_let(symbol_t *sym, ast_t *bound, ast_t *body, ast_t *type);

ast_t *ast_let_rec(func_def_t *func, ast_t *body);

ast_t *ast_var_ref(symbol_t *name);

ast_t *ast_int(int32_t value);

ast_t *ast_string(char *value);

ast_t *ast_binary_op(ast_t *left, ast_t *right, binary_op_t op);

ast_t *ast_if(ast_t *expr, ast_t *true_body, ast_t *false_body);

ast_t *ast_type_decl(char *name, symbol_t *symbol, ast_t *expr);

ast_t *ast_typed(ast_t *child, ast_t *type);

ast_t *ast_apply(ast_t *callee, GList *args);

ast_t *ast_tuple_type(GList *elem_types);

ast_t *ast_tuple(GList *elems);

func_def_t *mk_func_def(symbol_t *symbol, GList *params, ast_t *body, ast_t *ret_type);

symbol_t *mk_symbol(char *symbol);

param_t *mk_param(symbol_t *ident, ast_t *type);

void ast_free(ast_t *ast);

#endif //MLY_AST_H
