#include <stdbool.h>
#include <ast.h>

#include "type.h"
#include "g_list_utils.h"
#include "env.h"
#include "ast.h"
#include "infer.h"
#include "unify.h"

type_t *infer(env_t *env, ast_t *expr) {
  g_assert_nonnull(expr);

  switch (expr->ast_type) {
    case AST_UNIT:
      return mk_type(TYPE_UNIT);
    case AST_INT:
      return mk_type(TYPE_INT);
    case AST_BOOL:
      return mk_type(TYPE_BOOL);
    case AST_STRING:
      return mk_type(TYPE_STRING);
    case AST_BINARY_OP: {
      switch (BINARY_OP(expr).op) {
        case MULTIPLICATIVE_MOD:
        case MULTIPLICATIVE_DIV:
        case MULTIPLICATIVE_MUL:
        case ADDITIVE_SUB:
        case ADDITIVE_ADD: {
          type_t *l = infer(env, BINARY_OP(expr).left);
          type_t *r = infer(env, BINARY_OP(expr).right);
          if (!unify(l, r)) {
            g_error("Cannot unary op types of different values!");
          }
          return l;
        }
        case EQUALITY_EQUAL:
        case EQUALITY_NOT_EQUAL:
        case RELATIONAL_LESS:
        case RELATIONAL_LESS_EQUAL:
        case RELATIONAL_GREATER:
        case RELATIONAL_GREATER_EQUAL:
        case LOGICAL_AND:
        case LOGICAL_OR: {
          type_t *l = infer(env, BINARY_OP(expr).left);
          type_t *r = infer(env, BINARY_OP(expr).right);
          if (!unify(l, r)) {
            g_error("Cannot do equality check on types of different values!");
          }
          return mk_type(TYPE_BOOL);
        }
      }
    }
    case AST_IF: {
      type_t *exp = infer(env, IF(expr).expr);
      type_t *true_t = infer(env, IF(expr).true_body);
      type_t *false_t = infer(env, IF(expr).false_body);

      g_assert_cmpuint(exp->type, ==, TYPE_BOOL);

      if (!unify(true_t, false_t)) {
        g_error("both branches of an if/else clause must match (%s vs %s)",
                type_string(true_t), type_string(false_t));
      }

      return true_t;
    }
    case AST_APPLY: {
      __block GList *args = NULL;

      g_list_iterate(APPLY(expr).args, (g_list_iterate_cb) ^(ast_t *ast) {
        type_t *t = infer(env, ast);
        args = g_list_append(args, t);
      });

      type_t *ret = mk_type(TYPE_VAR);

      type_t *f = mk_type(TYPE_FUN);
      f->data.type_fun.ret = ret;
      f->data.type_fun.params = args;

      type_t *callee = infer(env, APPLY(expr).callee);

      if (!unify(callee, f)) {
        char *fn = VAR_REF(APPLY(expr).callee).symbol->name;
        g_error("Failed to unify type of called function %s %s vs %s", fn,
                type_string(callee), type_string(f));
      }

      return ret;
    }
    case AST_LET: {
      type_t *bound = infer(env, LET(expr).bound);
      if (LET(expr).type) {
        // TODO: Check non-inferred type.
      }
      env_set(env, LET(expr).symbol->name, bound);
      return infer(env, LET(expr).body);
    }
    case AST_LET_REC: {
      type_t *f = mk_type(TYPE_VAR);
      env_set(env, LET_REC(expr).func->symbol->name, f);

      __block GList *params = NULL;
      g_list_iterate(LET_REC(expr).func->params, (g_list_iterate_cb) ^(param_t *param) {
        type_t *p = NULL;
        if (param->type != NULL) {
          p = infer(env, param->type);
        } else {
          p = mk_type(TYPE_VAR);
        }

        env_set(env, param->ident->name, p);

        params = g_list_append(params, p);
      });

      type_t *ret = infer(env, LET_REC(expr).func->body);

      type_t *fun = mk_type(TYPE_FUN);
      fun->data.type_fun.params = params;
      fun->data.type_fun.ret = ret;

      if (!unify(fun, f)) {
        g_error("Failed to unify function");
      }

      return infer(env, LET_REC(expr).body);
    }
    case AST_VAR_REF: {
      type_t *typ = env_get(env, VAR_REF(expr).symbol->name);
      if (typ != NULL) {
        return typ;
      }

      external_t *ext = builtin_get(env, VAR_REF(expr).symbol->name);
      if (ext != NULL) {
        return ext->type;
      }

      printf("Unknown symbol: %s\n", VAR_REF(expr).symbol->name);
      abort();
      return NULL;
    }
    case AST_TUPLE: {
      __block GList *elems = NULL;
      g_list_iterate(TUPLE(expr).elems, (g_list_iterate_cb) ^(ast_t *ast) {
        elems = g_list_append(elems, infer(env, ast));
      });

      type_t *ret = mk_type(TYPE_TUPLE);
      ret->data.type_tuple.elems = elems;

      return ret;
    }
    default:
      printf("Failing to infer %i", expr->ast_type);
      return mk_type(TYPE_UNIT);
  }
}
