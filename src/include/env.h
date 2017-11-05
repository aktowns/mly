#ifndef MLY_ENV_H
#define MLY_ENV_H

#include "type.h"

typedef struct {
  type_t *type;
  char *cname;
} external_t;

typedef struct {
  GHashTable *table;
  GHashTable *externals;
} env_t;

env_t *mk_env();

void env_free(env_t* env);

void env_set(env_t *env, char *key, type_t *type);

type_t *env_get(env_t *env, char *key);

void builtin_set(env_t *env, char *key, external_t *type);

external_t *builtin_get(env_t *env, char *key);

#endif //MLY_ENV_H
