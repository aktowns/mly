#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <gc.h>
#include <glib.h>

#include "token.h"
#include "parser.h"

%%{
    machine mly;

    newline = '\n' @{ ++line_counter; line_position = (intptr_t)p; };

    comment := any* :>> '*)' @{ fgoto main; };

    number = ('+'|'-')?[0-9]+;
    ident = (lower | '_') (alnum | '_')*;

    main := |*
        '('     => { TOKEN(LPAREN);        };
        ')'     => { TOKEN(RPAREN);        };
        'true'  => {
            TOKEN(BOOL);
            cur_token->bool_value = true;
        };
        'false' => {
            TOKEN(BOOL);
            cur_token->bool_value = false;
        };
        'not'   => { TOKEN(NOT);           };
        number  => {
            char* dupd = GC_strdup(ts);
            dupd[te-ts] = '\0';
            int64_t val = strtol(dupd, NULL, 10);
            TOKEN(INT);
            cur_token->number_value = val;
            GC_FREE(dupd);
        };
        '-'     => { TOKEN(MINUS);         };
        '+'     => { TOKEN(PLUS);          };
        '*'     => { TOKEN(AST);           };
        '/'     => { TOKEN(SLASH);         };
        '%'     => { TOKEN(PERCENT);       };
        '-.'    => { TOKEN(MINUS_DOT);     };
        '+.'    => { TOKEN(PLUS_DOT);      };
        '*.'    => { TOKEN(AST_DOT);       };
        '/.'    => { TOKEN(SLASH_DOT);     };
        '='     => { TOKEN(EQUAL);         };
        '<>'    => { TOKEN(LESS_GREATER);  };
        '<='    => { TOKEN(LESS_EQUAL);    };
        '>='    => { TOKEN(GREATER_EQUAL); };
        '<'     => { TOKEN(LESS);          };
        '>'     => { TOKEN(GREATER);       };
        'if'    => { TOKEN(IF);            };
        'then'  => { TOKEN(THEN);          };
        'else'  => { TOKEN(ELSE);          };
        'let'   => { TOKEN(LET);           };
        'in'    => { TOKEN(IN);            };
        'rec'   => { TOKEN(REC);           };
        ','     => { TOKEN(COMMA);         };
        '_'     => { TOKEN(IDENT);         };
        'Array.create' => { TOKEN(ARRAY_MAKE); };
        'Array.make'   => { TOKEN(ARRAY_MAKE); };
        '.'     => { TOKEN(DOT);           };
        '<-'    => { TOKEN(LESS_MINUS);    };
        ';'     => { TOKEN(SEMICOLON);     };
        ident   => {
            char* substr = GC_strdup(ts);
            substr[te-ts] = '\0';
            char* duped = GC_strdup(substr);
            TOKEN(IDENT);
            cur_token->ident_value = duped;
            GC_FREE(substr);
        };

       	dstr_literal = [^"\\] | newline | ( '\\' (any | newline) );
    	'"' . dstr_literal* . '"' {
            char* substr = GC_strdup(ts);
            substr[te-ts] = '\0';
            char* duped = GC_strdup(substr);
            TOKEN(STRING_LITERAL);
            cur_token->ident_value = duped;
            GC_FREE(substr);
        };
    	'(*' { fgoto comment; };
       	[\t\v\f\r ]+;
        newline;
    *|;
}%%

#pragma clang diagnostic push
#if defined(__has_warning) && __has_warning("-Wunused-const-variable")
#pragma clang diagnostic ignored "-Wunused-const-variable"
#endif
%% write data nofinal;
#pragma clang diagnostic pop

#define TOKEN(t) \
  token_t* tok = mk_token(TOKEN_ ## t, line_counter, (intptr_t)(ts - line_position) + 1, lex_line(bfr, line_counter)); \
  if (cur_token != NULL) { \
    cur_token->next = tok; \
    cur_token = cur_token->next; \
  } else { \
    tokens = tok; \
    cur_token = tok; \
  }

char* lex_line(char* bfr, int line) {
    char** ary = g_strsplit(bfr, "\n", -1);
    char* ret = GC_strdup(ary[line - 1]);
    g_strfreev(ary);
    return ret;
}

token_t* lex(char* bfr) {
  if (bfr == NULL) return NULL;

  int line_counter = 1;
  intptr_t line_position = (intptr_t)bfr;
  size_t len = strlen(bfr);
  char *p = bfr;
  char *pe = p + len;
  char *eof = pe;
  int cs = 0, act = 0;
  char *ts, *te;

  %%write init;

  token_t *tokens = NULL;
  token_t *cur_token = tokens;

  %%write exec;

  return tokens;
}
