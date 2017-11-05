#ifndef MLY_TYPE_H
#define MLY_TYPE_H

#include <glib.h>
#include <stdio.h>

typedef enum {
  TYPE_UNIT,
  TYPE_BOOL,
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_STRING,
  TYPE_FUN,
  TYPE_TUPLE,
  TYPE_ARRAY,
  TYPE_OPTION,
  TYPE_VAR
} type_type_t;

#define CHECK_TYPE(x, ty) g_assert((x)->type == (ty))

#define TY_FUN(x)    (x)->data.type_fun
#define TY_TUPLE(x)  (x)->data.type_tuple
#define TY_ARRAY(x)  (x)->data.type_array
#define TY_OPTION(x) (x)->data.type_option
#define TY_VAR(x)    (x)->data.type_var

typedef struct typ {
  type_type_t type;

  union {
    struct {
      struct typ *ret;
      GList *params;
    } type_fun;
    struct {
      GList *elems;
    } type_tuple;
    struct {
      struct typ *elem;
    } type_array;
    struct {
      struct typ *elem;
    } type_option;
    struct {
      struct typ *ref;
    } type_var;
  } data;
} type_t;

typedef void (^type_iterate_cb)(type_t *);

char *type_string(type_t *typ);

type_t *mk_type(type_type_t tt);

void type_free(type_t *type);

#endif //MLY_TYPE_H
