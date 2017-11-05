#include "g_list_utils.h"
#include "unify.h"

bool unifyFun(type_t *left, type_t *right) {
  if (!unify(TY_FUN(left).ret, TY_FUN(right).ret)) {
    g_error("On unifying functions return types of '%s' and '%s'", type_string(left), type_string(right));
    return false;
  }

  size_t len_left = g_list_length(TY_FUN(left).params);
  size_t len_right = g_list_length(TY_FUN(right).params);
  if (len_left != len_right) {
    g_error("Number of parameters of function does not match %i vs %i", len_left, len_right);
    return false;
  }

  g_list_iteratei(TY_FUN(left).params, (g_list_iteratei_cb) ^(uint32_t i, type_t *t) {
    type_t *r = g_list_nth_data(TY_FUN(right).params, i);
    if (!unify(t, r)) {
      g_error("failed to unify function params %s vs %s", type_string(t), type_string(r));
    }
  });

  return true;
}

bool unify(type_t *left, type_t *right) {
  switch (left->type) {
    case TYPE_UNIT:
    case TYPE_BOOL:
    case TYPE_INT:
    case TYPE_FLOAT:
    case TYPE_STRING: {
      if (left->type == right->type) {
        return true;
      }
    };
      break;
    case TYPE_TUPLE:
      return false;
    case TYPE_ARRAY:
      return false;
    case TYPE_OPTION:
      return false;
    case TYPE_FUN: {
      if (right->type == TYPE_FUN) {
        return unifyFun(left, right);
      }
    }
      break;
    default:
      break;
  }

  bool lok = left->type == TYPE_VAR;
  bool rok = right->type == TYPE_VAR;

  if ((lok && rok) && left == right) {
    return true;
  }

  if (lok && TY_VAR(left).ref != NULL) {
    return unify(TY_VAR(left).ref, right);
  }

  if (rok && TY_VAR(right).ref != NULL) {
    return unify(left, TY_VAR(right).ref);
  }

  if (lok) {
    TY_VAR(left).ref = right;
    return true;
  }

  if (rok) {
    TY_VAR(right).ref = left;
    return true;
  }

  return false;
}
