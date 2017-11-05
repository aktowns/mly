#include "g_hash_table_utils.h"

void g_hash_table_iterate(GHashTable *table, g_hash_table_iterate_cb cb) {
  g_assert_nonnull(table);

  GHashTableIter iter;
  gpointer key, value;

  g_hash_table_iter_init(&iter, table);
  while (g_hash_table_iter_next(&iter, &key, &value)) {
    cb(key, value);
  }
}
