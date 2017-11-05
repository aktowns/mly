#ifndef MLY_G_LIST_UTILS_H
#define MLY_G_LIST_UTILS_H

#include <glib.h>
#include <stdint.h>

typedef void (^g_list_iterate_cb)(gpointer data);
typedef void (^g_list_iteratei_cb)(uint32_t i, gpointer data);

void g_list_iterate(GList* list, g_list_iterate_cb cb);
void g_list_iteratei(GList* list, g_list_iteratei_cb cb);

#endif //MLY_G_LIST_UTILS_H
