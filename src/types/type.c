#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <type.h>
#include <gc.h>

#include "g_list_utils.h"
#include "type.h"

char *type_join(GList *typs, const char *join) {
  __block GString *r = g_string_new(NULL);

  g_list_iterate(typs, (g_list_iterate_cb) ^(type_t *n) {
    char *nt = type_string(n);

    if (strlen(r->str) == 0) {
      r = g_string_append(r, nt);
    } else {
      g_string_append_printf(r, "%s%s", join, nt);
    }

    g_free(nt);
  });

  return g_string_free(r, FALSE);
}

char *type_string(type_t *typ) {
  if (typ == NULL) return g_strdup("NULL");
  g_assert_nonnull(typ);
  g_assert_cmpint(typ->type, <, 10);

  switch (typ->type) {
    case TYPE_INT:
      return g_strdup("int");
    case TYPE_FLOAT:
      return g_strdup("float");
    case TYPE_UNIT:
      return g_strdup("unit");
    case TYPE_BOOL:
      return g_strdup("bool");
    case TYPE_STRING:
      return g_strdup("string");
    case TYPE_OPTION: {
      char *inner = type_string(TY_OPTION(typ).elem);
      char *ret = g_strdup_printf("%s option", inner);

      g_free(inner);
      return ret;
    }
    case TYPE_ARRAY: {
      char *inner = type_string(TY_ARRAY(typ).elem);
      char *ret = g_strdup_printf("%s array", inner);

      g_free(inner);
      return ret;
    }
    case TYPE_FUN: {
      char *leader = type_join(TY_FUN(typ).params, " -> ");
      char *end = type_string(TY_FUN(typ).ret);

      char* ret = g_strdup_printf("%s -> %s", leader, end);

      g_free(leader);
      g_free(end);

      return ret;
    }
    case TYPE_TUPLE: {
      char *elems = type_join(TY_TUPLE(typ).elems, " * ");

      char* ret = g_strdup_printf("(%s)", elems);

      g_free(elems);

      return ret;
    }
    case TYPE_VAR: {
      char *inner = type_string(TY_VAR(typ).ref);
      return inner;
    }
  }

  g_error("Unhandled type");
  return NULL;
}

type_t *mk_type(type_type_t tt) {
  g_debug("mk_type tt=%i", tt);
  g_assert_cmpint(tt, <, 10);

  type_t *typ = GC_MALLOC(sizeof(type_t));
  memset(typ, 0, sizeof(type_t));
  typ->type = tt;

  return typ;
}

void type_free(type_t *type) {
  switch (type->type) {
    case TYPE_UNIT:
    case TYPE_BOOL:
    case TYPE_INT:
    case TYPE_FLOAT:
    case TYPE_STRING:
      break;
    case TYPE_FUN:
      type_free(TY_FUN(type).ret);
      g_list_free_full(TY_FUN(type).params, (GDestroyNotify) type_free);
      break;
    case TYPE_TUPLE:
      g_list_free_full(TY_TUPLE(type).elems, (GDestroyNotify) type_free);
      break;
    case TYPE_ARRAY:
      type_free(TY_ARRAY(type).elem);
      break;
    case TYPE_OPTION:
      type_free(TY_OPTION(type).elem);
      break;
    case TYPE_VAR:
      type_free(TY_VAR(type).ref);
      break;
  }

  g_free(type);
}