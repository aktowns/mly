#include <stdint.h>
#include <string.h>

#include "ast_pp.h"
#include "env.h"
#include "ast.h"
#include "g_list_utils.h"
#include "infer.h"
#include "xmlnode.h"

#undef SHOW_LINE_DATA

char *ast_type(env_t *env, ast_t *ast) {
  type_t *typ = infer(env, ast);
  char *typs = type_string(typ);
  //free(typ);

  return typs;
}

char *pp_ast(GList *ast) {
  g_assert_nonnull(ast);

  env_t *env = mk_env();

  xml_node_t *node = xmlpp_list(env, ast);
  char *out = print_xml(node);
  node_free(node);

  env_free(env);

  return out;
}

xml_node_t *xmlpp_list(env_t *env, GList *root) {
  g_assert_nonnull(root);

  xml_node_t *node = mk_node("list");
  node_add_attribute(node, "kind", "list");

  g_list_iteratei(root, (g_list_iteratei_cb) ^(uint32_t i, ast_t *tree) {
    g_assert_nonnull(tree);

    char *n = g_strdup_printf("item-%i", i);
    node_add_named_child(node, n, xmlpp(env, tree));
    g_free(n);
  });

  return node;
}

xml_node_t *xmlpp_unit(env_t *env, ast_t *ast) {
  g_assert_nonnull(ast);

  char *type = ast_type(env, ast);

  xml_node_t *node = mk_node("unit");
  node_add_attribute(node, "type", type);
  node_add_attribute(node, "kind", "node");
  node_add_attribute(node, "line-number", g_strdup_printf("%i", ast->location.line_no));
  node_add_attribute(node, "column-number", g_strdup_printf("%i", ast->location.column_no));
#ifdef SHOW_LINE_DATA
  node_add_attribute(node, "line", g_strdup_printf("%s", ast->location.line));
#endif

  g_free(type);

  return node;
}

xml_node_t *xmlpp_bool(env_t *env, ast_t *ast) {
  g_assert_nonnull(ast);

  CHECK_AST_TYPE(ast, AST_BOOL);

  char *type = ast_type(env, ast);

  xml_node_t *node = mk_node("bool");
  node_add_attribute(node, "type", type);
  node_add_attribute(node, "kind", "node");
  node_add_attribute(node, "line-number", g_strdup_printf("%i", LOC(ast).line_no));
  node_add_attribute(node, "column-number", g_strdup_printf("%i", LOC(ast).column_no));
#ifdef SHOW_LINE_DATA
  node_add_attribute(node, "line", g_strdup_printf("%s", LOC(ast).line));
#endif

  if (BOOL(ast)) {
    node_add_child_text(node, "true");
  } else {
    node_add_child_text(node, "false");
  }

  g_free(type);

  return node;
}

xml_node_t *xmlpp(env_t *env, ast_t *ast) {
  g_assert_nonnull(env);
  g_assert_nonnull(ast);

  switch (ast->ast_type) {
    case AST_LET:
      return xmlpp_let(env, ast);
    case AST_UNIT:
      return xmlpp_unit(env, ast);
    case AST_BOOL:
      return xmlpp_bool(env, ast);
    case AST_LET_REC:
      return xmlpp_let_rec(env, ast);
    case AST_VAR_REF:
      return xmlpp_var_ref(env, ast);
    case AST_BINARY_OP:
      return xmlpp_binary_op(env, ast);
    case AST_INT:
      return xmlpp_int(env, ast);
    case AST_STRING:
      return xmlpp_string(env, ast);
    case AST_IF:
      return xmlpp_if(env, ast);
    case AST_TYPEDECL:
      return xmlpp_type_decl(env, ast);
    case AST_TYPED:
      return xmlpp_typed(env, ast);
    case AST_APPLY:
      return xmlpp_apply(env, ast);
    case AST_TUPLE_TYPE:
      return xmlpp_tuple_type(env, ast);
    case AST_TUPLE:
      return xmlpp_tuple(env, ast);
  }

  g_error("xmlpp unhandled");
}

xml_node_t *xmlpp_symbol(env_t *env, symbol_t *symbol) {
  xml_node_t *node = mk_node("symbol");
  node_add_attribute(node, "kind", "value");
  node_add_attribute(node, "line-number", g_strdup_printf("%i", symbol->location.line_no));
  node_add_attribute(node, "column-number", g_strdup_printf("%i", symbol->location.column_no));
#ifdef SHOW_LINE_DATA
  node_add_attribute(node, "line", g_strdup_printf("%s", symbol->location.line));
#endif

  xml_node_t *name = mk_node("name");
  node_add_child_text(name, symbol->name);
  node_add_child(node, name);
  return node;
}

xml_node_t *xmlpp_param(env_t *env, param_t *param) {
  g_assert_nonnull(param);

  xml_node_t *node = mk_node("param");
  node_add_attribute(node, "kind", "value");
  node_add_attribute(node, "line-number", g_strdup_printf("%i", param->location.line_no));
  node_add_attribute(node, "column-number", g_strdup_printf("%i", param->location.column_no));
#ifdef SHOW_LINE_DATA
  node_add_attribute(node, "line", g_strdup_printf("%s", param->location.line));
#endif

  if (param->type != NULL) {
    node_add_named_child(node, "type", xmlpp(env, param->type));
  }

  node_add_named_child(node, "ident", xmlpp_symbol(env, param->ident));

  return node;
}

xml_node_t *xmlpp_params(env_t *env, GList *params) {
  g_assert_nonnull(params);

  xml_node_t *node = mk_node("list");
  node_add_attribute(node, "kind", "list");

  g_list_iteratei(params, (g_list_iteratei_cb) ^(uint32_t i, param_t *param) {
    char *n = g_strdup_printf("item-%i", i);
    node_add_named_child(node, n, xmlpp_param(env, param));
    g_free(n);
  });

  return node;
}

xml_node_t *xmlpp_func_def(env_t *env, func_def_t *func) {
  g_assert_nonnull(func);

  xml_node_t *node = mk_node("func-def");
  node_add_attribute(node, "kind", "value");
  node_add_attribute(node, "line-number", g_strdup_printf("%i", LOC(func).line_no));
  node_add_attribute(node, "column-number", g_strdup_printf("%i", LOC(func).column_no));
#ifdef SHOW_LINE_DATA
  node_add_attribute(node, "line", g_strdup_printf("%s", LOC(func).line));
#endif

  if (func->ret_type != NULL) {
    node_add_named_child(node, "ret-type", xmlpp(env, func->ret_type));
  }

  node_add_named_child(node, "sym", xmlpp_symbol(env, func->symbol));
  node_add_named_child(node, "params", xmlpp_params(env, func->params));
  node_add_named_child(node, "body", xmlpp(env, func->body));

  return node;
}

xml_node_t *xmlpp_let(env_t *env, ast_t *ast) {
  g_assert_nonnull(env);
  g_assert_nonnull(ast);

  CHECK_AST_TYPE(ast, AST_LET);

  char *type = ast_type(env, ast);

  xml_node_t *node = mk_node("let");
  node_add_named_child(node, "sym", xmlpp_symbol(env, LET(ast).symbol));
  node_add_named_child(node, "bound", xmlpp(env, LET(ast).bound));
  node_add_named_child(node, "body", xmlpp(env, LET(ast).body));
  node_add_attribute(node, "type", type);
  node_add_attribute(node, "kind", "node");
  node_add_attribute(node, "line-number", g_strdup_printf("%i", LOC(ast).line_no));
  node_add_attribute(node, "column-number", g_strdup_printf("%i", LOC(ast).column_no));
#ifdef SHOW_LINE_DATA
  node_add_attribute(node, "line", g_strdup_printf("%s", LOC(ast).line));
#endif

  g_free(type);

  return node;
}

xml_node_t *xmlpp_let_rec(env_t *env, ast_t *ast) {
  g_assert_nonnull(ast);

  CHECK_AST_TYPE(ast, AST_LET_REC);

  char *type = ast_type(env, ast);

  xml_node_t *node = mk_node("let-rec");
  node_add_named_child(node, "func", xmlpp_func_def(env, LET_REC(ast).func));
  node_add_named_child(node, "body", xmlpp(env, LET_REC(ast).body));
  node_add_attribute(node, "type", type);
  node_add_attribute(node, "kind", "node");
  node_add_attribute(node, "line-number", g_strdup_printf("%i", LOC(ast).line_no));
  node_add_attribute(node, "column-number", g_strdup_printf("%i", LOC(ast).column_no));
#ifdef SHOW_LINE_DATA
  node_add_attribute(node, "line", g_strdup_printf("%s", LOC(ast).line));
#endif

  g_free(type);

  return node;
}

xml_node_t *xmlpp_var_ref(env_t *env, ast_t *ast) {
  g_assert_nonnull(ast);

  CHECK_AST_TYPE(ast, AST_VAR_REF);

  char *type = ast_type(env, ast);

  xml_node_t *node = mk_node("var-ref");
  node_add_named_child(node, "sym", xmlpp_symbol(env, VAR_REF(ast).symbol));
  node_add_attribute(node, "type", type);
  node_add_attribute(node, "kind", "node");
  node_add_attribute(node, "line-number", g_strdup_printf("%i", LOC(ast).line_no));
  node_add_attribute(node, "column-number", g_strdup_printf("%i", LOC(ast).column_no));
#ifdef SHOW_LINE_DATA
  node_add_attribute(node, "line", g_strdup_printf("%s", LOC(ast).line));
#endif

  g_free(type);

  return node;
}

xml_node_t *xmlpp_int(env_t *env, ast_t *ast) {
  g_assert_nonnull(ast);

  CHECK_AST_TYPE(ast, AST_INT);

  char *type = ast_type(env, ast);

  xml_node_t *node = mk_node("int");
  node_add_attribute(node, "type", type);
  node_add_attribute(node, "kind", "node");
  node_add_attribute(node, "line-number", g_strdup_printf("%i", LOC(ast).line_no));
  node_add_attribute(node, "column-number", g_strdup_printf("%i", LOC(ast).column_no));
#ifdef SHOW_LINE_DATA
  node_add_attribute(node, "line", g_strdup_printf("%s", LOC(ast).line));
#endif

  xml_node_t *value = mk_node("value");
  node_add_child_text(value, g_strdup_printf("%i", INT(ast)));
  node_add_child(node, value);

  g_free(type);

  return node;
}

xml_node_t *xmlpp_string(env_t *env, ast_t *ast) {
  g_assert_nonnull(ast);

  CHECK_AST_TYPE(ast, AST_STRING);

  char *type = ast_type(env, ast);

  xml_node_t *node = mk_node("string");

  node_add_attribute(node, "type", type);
  node_add_attribute(node, "kind", "node");
  node_add_attribute(node, "line-number", g_strdup_printf("%i", LOC(ast).line_no));
  node_add_attribute(node, "column-number", g_strdup_printf("%i", LOC(ast).column_no));
#ifdef SHOW_LINE_DATA
  node_add_attribute(node, "line", g_strdup_printf("%s", LOC(ast).line));
#endif

  xml_node_t *value = mk_node("value");
  node_add_child_text(value, STR(ast));
  node_add_child(node, value);

  g_free(type);

  return node;
}

xml_node_t *xmlpp_if(env_t *env, ast_t *ast) {
  g_assert_nonnull(ast);

  CHECK_AST_TYPE(ast, AST_IF);

  char *type = ast_type(env, ast);

  xml_node_t *node = mk_node("if");
  node_add_named_child(node, "expr", xmlpp(env, IF(ast).expr));
  node_add_named_child(node, "true-body", xmlpp(env, IF(ast).true_body));
  node_add_named_child(node, "false-body", xmlpp(env, IF(ast).false_body));
  node_add_attribute(node, "type", type);
  node_add_attribute(node, "kind", "node");
  node_add_attribute(node, "line-number", g_strdup_printf("%i", LOC(ast).line_no));
  node_add_attribute(node, "column-number", g_strdup_printf("%i", LOC(ast).column_no));
#ifdef SHOW_LINE_DATA
  node_add_attribute(node, "line", g_strdup_printf("%s", LOC(ast).line));
#endif

  g_free(type);

  return node;
}

xml_node_t *xmlpp_type_decl(env_t *env, ast_t *ast) {
  g_assert_null(ast);

  CHECK_AST_TYPE(ast, AST_TYPEDECL);

  xml_node_t *node = mk_node("type-decl");
  node_add_named_child(node, "sym", xmlpp_symbol(env, TYPE_DECL(ast).symbol));
  node_add_named_child(node, "expr", xmlpp(env, TYPE_DECL(ast).expr));
  node_add_attribute(node, "kind", "node");
  node_add_attribute(node, "line-number", g_strdup_printf("%i", LOC(ast).line_no));
  node_add_attribute(node, "column-number", g_strdup_printf("%i", LOC(ast).column_no));
#ifdef SHOW_LINE_DATA
  node_add_attribute(node, "line", g_strdup_printf("%s", LOC(ast).line));
#endif

  xml_node_t *name = mk_node("name");
  node_add_child_text(name, TYPE_DECL(ast).name);
  node_add_child(node, name);

  return node;
}

xml_node_t *xmlpp_typed(env_t *env, ast_t *ast) {
  g_assert_null(ast);

  CHECK_AST_TYPE(ast, AST_TYPED);

  xml_node_t *node = mk_node("typed");
  node_add_named_child(node, "child", xmlpp(env, TYPED(ast).child));
  node_add_named_child(node, "type", xmlpp(env, TYPED(ast).type));
  node_add_attribute(node, "kind", "node");
  node_add_attribute(node, "line-number", g_strdup_printf("%i", LOC(ast).line_no));
  node_add_attribute(node, "column-number", g_strdup_printf("%i", LOC(ast).column_no));
#ifdef SHOW_LINE_DATA
  node_add_attribute(node, "line", g_strdup_printf("%s", LOC(ast).line));
#endif

  return node;
}

xml_node_t *xmlpp_apply(env_t *env, ast_t *ast) {
  g_assert_nonnull(ast);

  CHECK_AST_TYPE(ast, AST_APPLY);

  char *type = ast_type(env, ast);

  xml_node_t *node = mk_node("apply");
  node_add_named_child(node, "callee", xmlpp(env, APPLY(ast).callee));
  node_add_named_child(node, "args", xmlpp_list(env, APPLY(ast).args));
  node_add_attribute(node, "type", type);
  node_add_attribute(node, "kind", "node");
  node_add_attribute(node, "line-number", g_strdup_printf("%i", LOC(ast).line_no));
  node_add_attribute(node, "column-number", g_strdup_printf("%i", LOC(ast).column_no));
#ifdef SHOW_LINE_DATA
  node_add_attribute(node, "line", g_strdup_printf("%s", LOC(ast).line));
#endif

  g_free(type);

  return node;
}

xml_node_t *xmlpp_tuple_type(env_t *env, ast_t *ast) {
  g_assert_nonnull(ast);

  CHECK_AST_TYPE(ast, AST_TUPLE_TYPE);

  xml_node_t *node = mk_node("tuple-type");
  node_add_attribute(node, "kind", "node");
  node_add_attribute(node, "line-number", g_strdup_printf("%i", LOC(ast).line_no));
  node_add_attribute(node, "column-number", g_strdup_printf("%i", LOC(ast).column_no));
#ifdef SHOW_LINE_DATA
  node_add_attribute(node, "line", g_strdup_printf("%s", LOC(ast).line));
#endif

  g_list_iterate(TUPLE_TYPE(ast).elem_types, (g_list_iterate_cb) ^(ast_t *ast) {
    node_add_child(node, xmlpp(env, ast));
  });

  return node;
}

xml_node_t *xmlpp_tuple(env_t *env, ast_t *ast) {
  g_assert_nonnull(ast);

  CHECK_AST_TYPE(ast, AST_TUPLE);

  char *type = ast_type(env, ast);

  xml_node_t *node = mk_node("tuple");
  node_add_attribute(node, "kind", "node");
  node_add_attribute(node, "line-number", g_strdup_printf("%i", LOC(ast).line_no));
  node_add_attribute(node, "column-number", g_strdup_printf("%i", LOC(ast).column_no));
#ifdef SHOW_LINE_DATA
  node_add_attribute(node, "line", g_strdup_printf("%s", LOC(ast).line));
#endif

  xml_node_t *elems = mk_node("elems");
  g_list_iterate(TUPLE(ast).elems, (g_list_iterate_cb) ^(ast_t *ast) {
    node_add_child(elems, xmlpp(env, ast));
  });
  node_add_child(node, elems);

  node_add_attribute(node, "type", type);

  g_free(type);

  return node;
}

const char *binary_op_string(binary_op_t op) {
  switch (op) {
    case ADDITIVE_ADD:
      return "add";
    case ADDITIVE_SUB:
      return "subtraction";
    case EQUALITY_EQUAL:
      return "equal";
    case EQUALITY_NOT_EQUAL:
      return "not-equal";
    case RELATIONAL_LESS:
      return "less";
    case RELATIONAL_LESS_EQUAL:
      return "less-equal";
    case RELATIONAL_GREATER:
      return "greater";
    case RELATIONAL_GREATER_EQUAL:
      return "greater-equal";
    case MULTIPLICATIVE_MUL:
      return "multiplication";
    case MULTIPLICATIVE_DIV:
      return "division";
    case MULTIPLICATIVE_MOD:
      return "modulo";
    case LOGICAL_AND:
      return "and";
    case LOGICAL_OR:
      return "or";
  }

  g_error("Unhandled operand %i", op);
  return "unknown op";
}

xml_node_t *xmlpp_binary_op(env_t *env, ast_t *ast) {
  g_assert_nonnull(ast);

  CHECK_AST_TYPE(ast, AST_BINARY_OP);

  char *type = ast_type(env, ast);

  xml_node_t *node = mk_node("binary-op");

  node_add_named_child(node, "left", xmlpp(env, BINARY_OP(ast).left));
  node_add_named_child(node, "right", xmlpp(env, BINARY_OP(ast).right));

  xml_node_t *op = mk_node("op");
  node_add_child_text(op, (char *) binary_op_string(BINARY_OP(ast).op));
  node_add_child(node, op);

  node_add_attribute(node, "type", type);
  node_add_attribute(node, "kind", "node");
  node_add_attribute(node, "line-number", g_strdup_printf("%i", LOC(ast).line_no));
  node_add_attribute(node, "column-number", g_strdup_printf("%i", LOC(ast).column_no));
#ifdef SHOW_LINE_DATA
  node_add_attribute(node, "line", g_strdup_printf("%s", LOC(ast).line));
#endif

  g_free(type);

  return node;
}
