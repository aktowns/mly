#ifndef MLY_INFER_H
#define MLY_INFER_H

#include "ast.h"
#include "env.h"
#include "type.h"

type_t *infer(env_t *env, ast_t *expr);

#endif //MLY_INFER_H
