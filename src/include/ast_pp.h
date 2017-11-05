#ifndef MLY_AST_PP_H
#define MLY_AST_PP_H

#include <glib.h>

#include "ast.h"
#include "env.h"
#include "xmlnode.h"

xml_node_t *xmlpp(env_t *, ast_t *);

xml_node_t *xmlpp_list(env_t *, GList *);

xml_node_t *xmlpp_if(env_t *env, ast_t *ast);

xml_node_t *xmlpp_binary_op(env_t *env, ast_t *ast);

xml_node_t *xmlpp_string(env_t *env, ast_t *ast);

xml_node_t *xmlpp_int(env_t *env, ast_t *ast);

xml_node_t *xmlpp_var_ref(env_t *env, ast_t *ast);

xml_node_t *xmlpp_let_rec(env_t *env, ast_t *ast);

xml_node_t *xmlpp_unit(env_t *env, ast_t *ast);

xml_node_t *xmlpp_bool(env_t *env, ast_t *ast);

xml_node_t *xmlpp_let(env_t *env, ast_t *ast);

xml_node_t *xmlpp_type_decl(env_t *env, ast_t *ast);

xml_node_t *xmlpp_typed(env_t *env, ast_t *ast);

xml_node_t *xmlpp_apply(env_t *env, ast_t *ast);

xml_node_t *xmlpp_tuple_type(env_t *env, ast_t *ast);

xml_node_t *xmlpp_tuple(env_t *env, ast_t *ast);

xml_node_t *xmlpp_func_def(env_t *env, func_def_t *func);

xml_node_t *xmlpp_symbol(env_t *env, symbol_t *sym);

xml_node_t *xmlpp_param(env_t *env, param_t *param);

xml_node_t *xmlpp_params(env_t *env, GList *params);

#endif //MLY_AST_PP_H
