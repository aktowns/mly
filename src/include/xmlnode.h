#ifndef MLY_XMLNODE_H
#define MLY_XMLNODE_H

#include <glib.h>

typedef struct {
  char *name;
  GHashTable *attributes;
  GList *children;
  GString *text;
} xml_node_t;

xml_node_t *mk_node(char *name);

char *print_xml(xml_node_t *node);

void node_free(xml_node_t *node);

void node_add_child(xml_node_t *parent, xml_node_t *node);

void node_add_child_text(xml_node_t *parent, char *text);

void node_add_named_child(xml_node_t *parent, char *name, xml_node_t *node);

void node_add_attribute(xml_node_t *parent, char *key, char *value);

#endif //MLY_XMLNODE_H
