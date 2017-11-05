#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <xmlnode.h>
#include <gc.h>

#include "g_list_utils.h"
#include "g_hash_table_utils.h"
#include "xmlnode.h"

char *print_xml(xml_node_t *node) {
  g_assert_nonnull(node);

  GString *str_node = g_string_new(NULL);

  __block GString *str_attributes = g_string_new(NULL);

  g_hash_table_iterate(node->attributes, (g_hash_table_iterate_cb) ^(char *key, char *value) {
    g_string_append_printf(str_attributes, " %s=\"%s\"", key, value);
  });

  if (g_list_length(node->children) > 0 || strlen(node->text->str) > 0) {
    __block GString *str_child = g_string_new(node->text->str);

    g_list_iterate(node->children, (g_list_iterate_cb) ^(xml_node_t *child_node) {
      g_assert_nonnull(child_node);

      char* child_str = print_xml(child_node);
      g_string_append(str_child, child_str);
      free(child_str);
    });

    g_string_printf(str_node, "<%s%s>%s</%s>", node->name, str_attributes->str, str_child->str, node->name);

    g_string_free(str_child, TRUE);
  } else {
    g_string_printf(str_node, "<%s%s />", node->name, str_attributes->str);
  }

  g_string_free(str_attributes, TRUE);

  return g_string_free(str_node, FALSE);
}

xml_node_t *mk_node(char *name) {
  xml_node_t *node = GC_MALLOC(sizeof(xml_node_t));
  node->name = GC_strdup(name);
  node->attributes = g_hash_table_new(g_str_hash, g_str_equal);
  node->children = NULL;
  node->text = g_string_new(NULL);
  return node;
}

void node_free(xml_node_t *node) {
  printf("Freeing node: %p\n", node);

  GC_FREE(node->name);
  g_string_free(node->text, TRUE);
  g_hash_table_remove_all(node->attributes);
  g_list_iterate(node->children, (g_list_iterate_cb) ^(xml_node_t *child_node) {
    node_free(child_node);
  });

  GC_FREE(node);
}

void node_add_child(xml_node_t *parent, xml_node_t *node) {
  parent->children = g_list_append(parent->children, node);
}

void node_add_child_text(xml_node_t *parent, char *text) {
  parent->text = g_string_append(parent->text, g_strdup(text));
}

void node_add_named_child(xml_node_t *parent, char *name, xml_node_t *node) {
  xml_node_t *named_node = mk_node(name);
  node_add_child(named_node, node);

  parent->children = g_list_append(parent->children, named_node);
}

void node_add_attribute(xml_node_t *parent, char *key, char *value) {
  g_hash_table_insert(parent->attributes, key, g_strdup(value));
}
