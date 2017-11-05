#include "type.h"
#include "env.h"

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gc.h>
#include <ast.h>
#include <env.h>

external_t *external(char *name, GList *params, type_t *ret) {
  type_t *f = mk_type(TYPE_FUN);
  f->data.type_fun.ret = ret;
  f->data.type_fun.params = params;

  external_t *ext = GC_MALLOC(sizeof(external_t));
  ext->cname = name;
  ext->type = f;

  return ext;
}

env_t *mk_env() {
  env_t *env = GC_MALLOC(sizeof(env_t));

  env->table = g_hash_table_new_full(g_str_hash, g_str_equal, GC_free, GC_free);
  env->externals = g_hash_table_new_full(g_str_hash, g_str_equal, GC_free, GC_free);

  external_t *ext_print_int = external("print_int",
                                       g_list_append(NULL, mk_type(TYPE_INT)),
                                       mk_type(TYPE_UNIT));

  external_t *ext_println_str = external("println_str",
                                         g_list_append(NULL, mk_type(TYPE_STRING)),
                                         mk_type(TYPE_UNIT));

  external_t *ext_println_int = external("println_int",
                                         g_list_append(NULL, mk_type(TYPE_INT)),
                                         mk_type(TYPE_UNIT));

  builtin_set(env, "print_int", ext_print_int);
  builtin_set(env, "println_str", ext_println_str);
  builtin_set(env, "println_int", ext_println_int);

  return env;
}

void env_free(env_t* env) {
  g_hash_table_destroy(env->table);
  g_hash_table_destroy(env->externals);
  GC_FREE(env);
}

void env_set(env_t *env, char *key, type_t *type) {
  g_hash_table_insert(env->table, GC_strdup(key), type);
}

type_t *env_get(env_t *env, char *key) {
  return g_hash_table_lookup(env->table, key);
}

void builtin_set(env_t *env, char *key, external_t *type) {
  g_hash_table_insert(env->externals, GC_strdup(key), type);
}

external_t *builtin_get(env_t *env, char *key) {
  return g_hash_table_lookup(env->externals, key);
}
