#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ast.h>
#include <gc.h>

#include "g_list_utils.h"
#include "type.h"
#include "ast.h"
#include "infer.h"

location_t loc_from_token(token_t *token) {
  location_t loc;
  loc.column_no = token->column_no;
  loc.line_no = token->line_no;
  loc.line = token->line;
  return loc;
}

ast_t *mk_ast(ast_type_t type) {
  g_debug("mk_ast type=%i", type);

  ast_t *ast = GC_MALLOC(sizeof(ast_t));
  memset(ast, 0, sizeof(ast_t));

  ast->ast_type = type;
  ast->location.column_no = 0;
  ast->location.line_no = 0;

  return ast;
}

void ast_let_free(ast_t *ast);

void ast_let_rec_free(ast_t *ast);

void ast_var_ref_free(ast_t *ast);

void ast_binary_op_free(ast_t *ast);

void ast_if_free(ast_t *ast);

void ast_typedecl_free(ast_t *ast);

void ast_typed_free(ast_t *ast);

void ast_apply_free(ast_t *ast);

void ast_tuple_type_free(ast_t *ast);

void ast_tuple_free(ast_t *ast);

void func_def_free(func_def_t *fd);

void symbol_free(symbol_t *sym);

void param_free(param_t *param);

void ast_free(ast_t *ast) {
  printf("Freeing: %p\n", ast);
  g_assert_nonnull(ast);

  switch (ast->ast_type) {
    case AST_UNIT:
    case AST_BOOL:
    case AST_INT:
      break;
    case AST_STRING:
      g_free(STR(ast));
      break;
    case AST_LET:
      ast_let_free(ast);
      break;
    case AST_LET_REC:
      ast_let_rec_free(ast);
      break;
    case AST_VAR_REF:
      ast_var_ref_free(ast);
      break;
    case AST_BINARY_OP:
      ast_binary_op_free(ast);
      break;
    case AST_IF:
      ast_if_free(ast);
      break;
    case AST_TYPEDECL:
      ast_typedecl_free(ast);
      break;
    case AST_TYPED:
      ast_typed_free(ast);
      break;
    case AST_APPLY:
      ast_apply_free(ast);
      break;
    case AST_TUPLE_TYPE:
      ast_tuple_type_free(ast);
      break;
    case AST_TUPLE:
      ast_tuple_free(ast);
      break;
    default:
      abort();
  }

  GC_FREE(ast);
}

ast_t *ast_unit() {
  ast_t *ast = mk_ast(AST_UNIT);
  return ast;
}

ast_t *ast_bool(bool val) {
  ast_t *ast = mk_ast(AST_BOOL);

  BOOL(ast) = val;

  return ast;
}

ast_t *ast_let(symbol_t *sym, ast_t *bound, ast_t *body, ast_t *type) {
  g_assert_nonnull(sym);
  g_assert_nonnull(body);
  g_assert_nonnull(bound);

  ast_t *ast = mk_ast(AST_LET);

  LET(ast).symbol = sym;
  LET(ast).body = body;
  LET(ast).bound = bound;
  LET(ast).type = type;

  return ast;
}

void ast_let_free(ast_t *ast) {
  symbol_free(LET(ast).symbol);
  ast_free(LET(ast).bound);
  ast_free(LET(ast).body);

  if (LET(ast).type != NULL) {
    ast_free(LET(ast).type);
  }
}

ast_t *ast_let_rec(func_def_t *func, ast_t *body) {
  g_assert_nonnull(func);
  g_assert_nonnull(body);

  ast_t *ast = mk_ast(AST_LET_REC);

  LET_REC(ast).func = func;
  LET_REC(ast).body = body;

  return ast;
}

void ast_let_rec_free(ast_t *ast) {
  func_def_free(LET_REC(ast).func);
  ast_free(LET_REC(ast).body);
}

ast_t *ast_var_ref(symbol_t *name) {
  g_assert_nonnull(name);

  ast_t *ast = mk_ast(AST_VAR_REF);

  VAR_REF(ast).symbol = name;

  return ast;
}

void ast_var_ref_free(ast_t *ast) {
  symbol_free(VAR_REF(ast).symbol);
}

ast_t *ast_int(int32_t value) {
  ast_t *ast = mk_ast(AST_INT);

  INT(ast) = value;

  return ast;
}

ast_t *ast_string(char *value) {
  ast_t *ast = mk_ast(AST_STRING);

  STR(ast) = g_strdup(value);

  return ast;
}

ast_t *ast_binary_op(ast_t *left, ast_t *right, binary_op_t op) {
  g_assert_nonnull(left);
  g_assert_nonnull(right);

  ast_t *ast = mk_ast(AST_BINARY_OP);

  BINARY_OP(ast).left = left;
  BINARY_OP(ast).right = right;
  BINARY_OP(ast).op = op;

  return ast;
}

void ast_binary_op_free(ast_t *ast) {
  ast_free(BINARY_OP(ast).left);
  ast_free(BINARY_OP(ast).right);
}

ast_t *ast_if(ast_t *expr, ast_t *true_body, ast_t *false_body) {
  g_assert_nonnull(expr);
  g_assert_nonnull(true_body);
  g_assert_nonnull(false_body);

  ast_t *ast = mk_ast(AST_IF);

  IF(ast).expr = expr;
  IF(ast).true_body = true_body;
  IF(ast).false_body = false_body;

  return ast;
}

void ast_if_free(ast_t *ast) {
  ast_free(IF(ast).expr);
  ast_free(IF(ast).true_body);
  ast_free(IF(ast).false_body);
}

func_def_t *mk_func_def(symbol_t *symbol, GList *params, ast_t *body, ast_t *ret_type) {
  g_assert_nonnull(symbol);
  g_assert_nonnull(params);
  g_assert_nonnull(body);

  func_def_t *fd = GC_MALLOC(sizeof(func_def_t));
  fd->location.line_no = 0;
  fd->location.column_no = 0;

  fd->symbol = symbol;
  fd->params = params;
  fd->body = body;
  fd->ret_type = ret_type;

  return fd;
}

void func_def_free(func_def_t *fd) {
  symbol_free(fd->symbol);
  g_list_free_full(fd->params, (GDestroyNotify) param_free);
  ast_free(fd->body);
  if (fd->ret_type != NULL) {
    ast_free(fd->ret_type);
  }
  GC_FREE(fd);
}

symbol_t *mk_symbol(char *symbol) {
  g_assert_nonnull(symbol);

  symbol_t *sym = GC_MALLOC(sizeof(symbol_t));
  sym->location.line_no = 0;
  sym->location.column_no = 0;
  sym->name = g_strdup(symbol);

  return sym;
}

void symbol_free(symbol_t *sym) {
  printf("Freeing: %p\n", sym);
  g_free(sym->name);
  GC_FREE(sym);
}

param_t *mk_param(symbol_t *ident, ast_t *type) {
  g_assert_nonnull(ident);

  param_t *param = GC_MALLOC(sizeof(param_t));

  param->ident = ident;
  param->type = type;

  return param;
}

void param_free(param_t *param) {
  symbol_free(param->ident);
  if (param->type != NULL) {
    ast_free(param->type);
  }
}

ast_t *ast_type_decl(char *name, symbol_t *symbol, ast_t *expr) {
  g_assert_nonnull(name);
  g_assert_nonnull(symbol);
  g_assert_nonnull(expr);
  ast_t *ast = mk_ast(AST_TYPEDECL);

  TYPE_DECL(ast).name = name;
  TYPE_DECL(ast).symbol = symbol;
  TYPE_DECL(ast).expr = expr;

  return ast;
}

void ast_typedecl_free(ast_t *ast) {
  g_free(TYPE_DECL(ast).name);
  symbol_free(TYPE_DECL(ast).symbol);
  ast_free(TYPE_DECL(ast).expr);
}

ast_t *ast_typed(ast_t *child, ast_t *type) {
  g_assert_nonnull(child);
  g_assert_nonnull(type);

  ast_t *ast = mk_ast(AST_TYPED);

  TYPED(ast).child = child;
  TYPED(ast).type = type;

  return ast;
}

void ast_typed_free(ast_t *ast) {
  ast_free(TYPED(ast).child);
  ast_free(TYPED(ast).type);
}

ast_t *ast_apply(ast_t *callee, GList *args) {
  g_assert_nonnull(callee);
  g_assert_nonnull(args);

  ast_t *ast = mk_ast(AST_APPLY);

  APPLY(ast).callee = callee;
  APPLY(ast).args = args;

  return ast;
}

void ast_apply_free(ast_t *ast) {
  ast_free(APPLY(ast).callee);
  g_list_free_full(APPLY(ast).args, (GDestroyNotify) ast_free);
}

ast_t *ast_tuple_type(GList *elem_types) {
  g_assert_nonnull(elem_types);

  ast_t *ast = mk_ast(AST_TUPLE_TYPE);

  TUPLE_TYPE(ast).elem_types = elem_types;

  return ast;
}

void ast_tuple_type_free(ast_t *ast) {
  g_list_free_full(TUPLE_TYPE(ast).elem_types, (GDestroyNotify) ast_free);
}

ast_t *ast_tuple(GList *elems) {
  g_assert_nonnull(elems);

  ast_t *ast = mk_ast(AST_TUPLE);

  TUPLE(ast).elems = elems;

  return ast;
}

void ast_tuple_free(ast_t *ast) {
  g_list_free_full(TUPLE(ast).elems, (GDestroyNotify) ast_free);
}
