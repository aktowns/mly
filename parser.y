%include {
    #include <stdlib.h>
    #include <stdio.h>
    #include <assert.h>
    #include <stdbool.h>
    #include <string.h>

    #include "token.h"
    #include "ast.h"

    #pragma clang diagnostic ignored "-Wunused-variable"

    #define NI(node) _NI(node, __LINE__)

    static inline ast_t* _NI(char* node, int line) {
        fprintf(stderr, "%i:%s is not implemented", line, node);
        abort();
        return NULL;
    }
}

%syntax_error {
    printf("syntax error, expecting one of: ");
    int n = sizeof(yyTokenName) / sizeof(yyTokenName[0]);
    for (int i = 0; i < n; ++i) {
        int a = yy_find_shift_action(yypParser, (YYCODETYPE)i);
        if (a < YYNSTATE + YYNRULE) {
            printf("%s ", yyTokenName[i]);
        }
    }
    printf("\n");
}

%token_prefix TOKEN_
%token_type { token_t * }
%default_type { ast_t * }
%extra_argument { GList **result }

%start_symbol program

%nonassoc IN.
%right PREC_LET.
%right PREC_SEQ.
%right SEMICOLON.
%nonassoc WITH.
%right PREC_IF.
%right PREC_MATCH.
%right PREC_FUN.
%right LESS_MINUS.
%nonassoc BAR.
%left PREC_TUPLE.
%left COMMA.
%left BAR_BAR.
%left AND_AND.
%left EQUAL LESS_GREATER LESS GREATER LESS_EQUAL GREATER_EQUAL.
%left PLUS MINUS PLUS_DOT MINUS_DOT.
%left AST SLASH AST_DOT SLASH_DOT PERCENT.
%right PREC_UNARY_MINUS.
%left PREC_APP.
%left DOT.
%nonassoc PREC_BELOW_IDENT.
%nonassoc IDENT.

%type program { GList* }
program ::= toplevels(top) seq_exp(root). {
    *result = g_list_append(top, root);
}

%type toplevels { GList* }
toplevels(A) ::= . { A = NULL; }
toplevels(A) ::= toplevels TYPE IDENT EQUAL type SEMICOLON. { A = NULL; }
toplevels(A) ::= toplevels EXTERNAL IDENT COLON type EQUAL STRING_LITERAL SEMICOLON. { A = NULL; }

seq_exp(A) ::= exp(B). [PREC_SEQ] { A = B; }
seq_exp(A) ::= seq_exp(B) SEMICOLON exp(C). {
    symbol_t* sym = mk_symbol("_");
    sym->location = B->location;

    A = ast_let(sym, B, C, NULL);
    A->location = B->location;
}

exp(A) ::= simple_exp(B). { A = B; }
exp(A) ::= NOT exp. [PREC_APP] { A = NI("NOT"); }
exp(A) ::= MINUS exp. [PREC_UNARY_MINUS] { A = NI("MINUS"); }
exp(A) ::= exp(B) PLUS exp(C).          {
    A = ast_binary_op(B, C, ADDITIVE_ADD);
    A->location = B->location;
}
exp(A) ::= exp(B) MINUS exp(C).         {
    A = ast_binary_op(B, C, ADDITIVE_SUB);
    A->location = B->location;
}
exp(A) ::= exp(B) AST exp(C).           {
    A = ast_binary_op(B, C, MULTIPLICATIVE_MUL);
    A->location = B->location;
}
exp(A) ::= exp(B) SLASH exp(C).         {
    A = ast_binary_op(B, C, MULTIPLICATIVE_DIV);
    A->location = B->location;
}
exp(A) ::= exp(B) PERCENT exp(C).       {
    A = ast_binary_op(B, C, MULTIPLICATIVE_MOD);
    A->location = B->location;
}
exp(A) ::= exp(B) EQUAL exp(C).         {
    A = ast_binary_op(B, C, EQUALITY_EQUAL);
    A->location = B->location;
}
exp(A) ::= exp(B) LESS_GREATER exp(C).  {
    A = ast_binary_op(B, C, EQUALITY_NOT_EQUAL);
    A->location = B->location;
}
exp(A) ::= exp(B) LESS exp(C).          {
    A = ast_binary_op(B, C, RELATIONAL_LESS);
    A->location = B->location;
}
exp(A) ::= exp(B) GREATER exp(C).       {
    A = ast_binary_op(B, C, RELATIONAL_GREATER);
    A->location = B->location;
}
exp(A) ::= exp(B) LESS_EQUAL exp(C).    {
    A = ast_binary_op(B, C, RELATIONAL_LESS_EQUAL);
    A->location = B->location;
}
exp(A) ::= exp(B) GREATER_EQUAL exp(C). {
    A = ast_binary_op(B, C, RELATIONAL_GREATER_EQUAL);
    A->location = B->location;
}
exp(A) ::= exp(B) AND_AND exp(C).       {
    A = ast_binary_op(B, C, LOGICAL_AND);
    A->location = B->location;
}
exp(A) ::= exp(B) BAR_BAR exp(C).       {
    A = ast_binary_op(B, C, LOGICAL_OR);
    A->location = B->location;
}
exp(A) ::= IF(s) seq_exp(B) THEN seq_exp(C) ELSE exp(D). [PREC_IF] {
    A = ast_if(B, C, D);
    A->location = loc_from_token(s);
}
exp(A) ::= MATCH seq_exp match_arm_start SOME match_ident MINUS_GREATER seq_exp BAR NONE MINUS_GREATER exp. [PREC_MATCH] { A = NI("MATCH SOME"); }
exp(A) ::= MATCH seq_exp match_arm_start NONE MINUS_GREATER seq_exp BAR SOME match_ident MINUS_GREATER exp. [PREC_MATCH] { A = NI("MATCH_NONE"); }
exp(A) ::= MINUS_DOT exp. [PREC_UNARY_MINUS] { A = NI("MINUS_DOT"); }
exp(A) ::= exp PLUS_DOT exp.  { A = NI("PLUS_DOT"); }
exp(A) ::= exp MINUS_DOT exp. { A = NI("MINUS_DOT"); }
exp(A) ::= exp AST_DOT exp.   { A = NI("AST_DOT"); }
exp(A) ::= exp SLASH_DOT exp. { A = NI("SLASH_DOT"); }
exp(A) ::= LET(s) IDENT(B) type_annotation(C) EQUAL seq_exp(D) IN seq_exp(E). [PREC_LET] {
    symbol_t* sym = mk_symbol(B->ident_value);
    sym->location = loc_from_token(B);

    A = ast_let(sym, D, E, C);
    A->location = loc_from_token(s);
}
exp(A) ::= LET(s) REC fundef(B) IN seq_exp(C). [PREC_LET] {
    A = ast_let_rec(B,C);
    A->location = loc_from_token(s);
}
exp(A) ::= simple_exp(B) args(C). [PREC_APP] {
    A = ast_apply(B, C);
    A->location = B->location;
}
exp(A) ::= elems(B). [PREC_TUPLE] {
    A = ast_tuple(B);
}
exp(A) ::= LET LPAREN pat RPAREN type_annotation EQUAL seq_exp IN seq_exp. {
    A = NI("LET LPAREN");
}
exp(A) ::= simple_exp DOT LPAREN exp RPAREN LESS_MINUS exp. { A = NI("SIMPLE_EXP DOT LPAREN"); }
exp(A) ::= ARRAY_MAKE simple_exp simple_exp. [PREC_APP] { A = NI("ARRAY MAKE"); }
exp(A) ::= ARRAY_LENGTH simple_exp. [PREC_APP] { A = NI("ARRAY LENGTH"); }
exp(A) ::= SOME simple_exp. { A = NI("SOME"); }
exp(A) ::= FUN params simple_type_annotation MINUS_GREATER seq_exp. [PREC_FUN] { A = NI("FUN"); }
exp(A) ::= ILLEGAL error. { A = NI("ILLEGAL"); }

%type fundef { func_def_t* }
fundef(A) ::= IDENT(B) params(C) type_annotation(D) EQUAL seq_exp(E). {
    symbol_t* sym = mk_symbol(B->ident_value);
    sym->location = loc_from_token(B);

    A = mk_func_def(sym,C,E,D);
    A->location = loc_from_token(B);
}

%type params { GList* }
params(A) ::= IDENT(B). {
    symbol_t* sym = mk_symbol(B->string_value);
    sym->location = loc_from_token(B);

    param_t* param = mk_param(sym, NULL);
    param->location = loc_from_token(B);

    A = g_list_append(NULL, param);
}
params(A) ::= LPAREN IDENT(B) COLON type(C) RPAREN. {
    symbol_t* sym = mk_symbol(B->string_value);
    sym->location = loc_from_token(B);

    param_t* param = mk_param(sym, C);
    param->location = loc_from_token(B);

    A = g_list_append(NULL, param);
}
params(A) ::= params(B) IDENT(C). {
    symbol_t* sym = mk_symbol(C->string_value);
    sym->location = loc_from_token(C);

    param_t* param = mk_param(sym, NULL);
    param->location = loc_from_token(C);

    A = g_list_append(B, param);
}
params(A) ::= params(B) LPAREN IDENT(C) COLON type(D) RPAREN. {
    symbol_t* sym = mk_symbol(C->string_value);
    sym->location = loc_from_token(C);

    param_t* param = mk_param(sym, D);
    param->location = loc_from_token(C);

    A = g_list_append(B, param);
}

%type args { GList* }
args(A) ::= args(B) simple_exp(C). { A = g_list_append(B, C); }
args(A) ::= simple_exp(B). { A = g_list_append(NULL, B); }

%type elems { GList* }
elems(A) ::= elems(B) COMMA exp(C). { A = g_list_append(B, C); }
elems(A) ::= exp(B) COMMA exp(C). { A = g_list_append(g_list_append(NULL, B), C); }

%type pat { GList* }
pat(A) ::= pat(B) COMMA IDENT(C). {
    symbol_t* sym = mk_symbol(C->ident_value);
    sym->location = loc_from_token(C);

    A = g_list_append(B, sym);
}
pat(A) ::= IDENT(B) COMMA IDENT(C). {
    A = g_list_append(g_list_append(NULL, B), C);
}

simple_exp(A) ::= LPAREN seq_exp(B) type_annotation(C) RPAREN. {
    if (C == NULL) {
        A = B;
    } else {
        A = ast_typed(B, C);
    }
}
simple_exp(A) ::= LPAREN(B) RPAREN. {
    A = ast_unit();
    A->location = loc_from_token(B);
}
simple_exp(A) ::= BOOL(B). {
    A = ast_bool(B->bool_value);
    A->location = loc_from_token(B);
}
simple_exp(A) ::= INT(B). {
    A = ast_int(B->number_value);
    A->location = loc_from_token(B);
}
simple_exp(A) ::= FLOAT. { A = NI("FLOAT"); }
simple_exp(A) ::= STRING_LITERAL(B). {
    A = ast_string(B->string_value);
    A->location = loc_from_token(B);
}
simple_exp(A) ::= LBRACKET_BAR BAR_RBRACKET. { A = NI("LBRACKET BAR"); }
simple_exp(A) ::= LBRACKET_BAR semi_elems opt_semi BAR_RBRACKET. { A = NI("LBRACKET BAR SEMI ELEMS"); }
simple_exp(A) ::= LBRACKET RBRACKET error. { A = NI("KBRACKET RBRACKET ERROR"); }
simple_exp(A) ::= LRBACKET semi_elems opt_semi RBRACKET error. { A = NI("SEMI ELEMS OPT SEMI"); }
simple_exp(A) ::= NONE. { A = NI("NONE"); }
simple_exp(A) ::= IDENT(B). {
    symbol_t* sym = mk_symbol(B->ident_value);
    sym->location = loc_from_token(B);

    A = ast_var_ref(sym);
    A->location = loc_from_token(B);
}
simple_exp(A) ::= simple_exp DOT LPAREN exp RPAREN. { A = NI("SIMPLE EXP DOT LPAREN"); }

match_arm_start(A) ::= WITH BAR. { A = NI("WITH BAR"); }
match_arm_start(A) ::= WITH. { A = NI("WITH"); }

match_ident(A) ::= LPAREN IDENT RPAREN. { A = NI("LPAREN IDENT"); }
match_ident(A) ::= IDENT. { A = NI("IDENT"); }

%type semi_elems { GList* }
semi_elems(A) ::= exp(B). [PREC_SEQ] { A = g_list_append(NULL, B); }
semi_elems(A) ::= semi_elems(B) SEMICOLON exp(C). { A = g_list_append(B, C); }

opt_semi ::= SEMICOLON. { }

type_annotation(A) ::= . { A = NULL; }
type_annotation(A) ::= COLON type(B). { A = B; }

simple_type_annotation(A) ::= . { A = NULL; }
simple_type_annotation(A) ::= COLON simple_type(B). { A = B; }

type(A) ::= simple_type_or_tuple. { A = NI("SIMPLE TYPE OR TUPLE"); }
type(A) ::= simple_type_or_tuple MINUS_GREATER arrow_types. { A = NI("SIMPLE TYPE OR TUPLE MINUS GREATER"); }

%type arrow_types { GList* }
arrow_types(A) ::= simple_type_or_tuple(B). { A = g_list_append(NULL, B); }
arrow_types(A) ::= arrow_types(B) MINUS_GREATER simple_type_or_tuple(C). { A = g_list_append(B, C); }

simple_type_or_tuple(A) ::= simple_type(B). { A = B; }
simple_type_or_tuple(A) ::= simple_type(B) AST simple_type_star_list(C). {
    A = ast_tuple_type(g_list_append(g_list_append(NULL, B), C));
    A->location = B->location;
}

%type simple_type_star_list { GList* }
simple_type_star_list(A) ::= simple_type(B). { A = g_list_append(NULL, B); }
simple_type_star_list(A) ::= simple_type_star_list(B) AST simple_type(C). {
    A = g_list_append(B, C);
}

simple_type(A) ::= IDENT. { A = NI("IDENT"); }
simple_type(A) ::= simple_type IDENT. { A = NI("SIMPLE TYPE IDENT"); }
simple_type(A) ::= LPAREN type_comma_list RPAREN IDENT. { A = NI("LAPREN TYPE COMMA LIST IDENT"); }
simple_type(A) ::= LPAREN type_comma_list RPAREN. [PREC_BELOW_IDENT] { A = NI("LPAREN TYPE COMMA LIST"); }

%type type_comma_list { GList* }
type_comma_list(A) ::= type(B). { A = g_list_append(NULL, B); }
type_comma_list(A) ::= type_comma_list(B) COMMA type(C). {
    A = g_list_append(B, C);
}

