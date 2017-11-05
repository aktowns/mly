#ifndef MLY_G_HASH_TABLE_UTILS_H
#define MLY_G_HASH_TABLE_UTILS_H

#include <glib.h>

typedef void (^g_hash_table_iterate_cb)(gpointer key, gpointer value);

void g_hash_table_iterate(GHashTable *table, g_hash_table_iterate_cb cb);

#endif //MLY_G_HASH_TABLE_UTILS_H
