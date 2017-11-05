#include <stdio.h>
#include "g_list_utils.h"

void g_list_iterate(GList *list, g_list_iterate_cb cb) {
  GList *l;

  for (l = list; l != NULL; l = l->next) {
    g_assert_nonnull(l->data);
    cb(l->data);
  }
}

void g_list_iteratei(GList *list, g_list_iteratei_cb cb) {
  GList *l;

  __block uint32_t i = 0;
  for (l = list; l != NULL; l = l->next) {
    g_assert_nonnull(l->data);
    cb(i, l->data);
    i += 1;
  }
}
