#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gc.h>

#include "token.h"
#include "ast.h"

void *ParseAlloc(void *);

void Parse(void *, int, void *, void **);

void ParseTrace(FILE *, char *);

void ParseFree(void *, void *);

int main(int argc, char **argv) {
  printf("mly compiler 0.000000000001\n");

  if (argc == 1) {
    printf("%s [filename]\n", argv[ 0 ]);
    exit(-1);
  }

  char *data = NULL;
  g_file_get_contents(argv[ 1 ], &data, NULL, NULL);

  token_t *tokens = lex(data);
  // print_tokens(tokens);

  // ParseTrace(stderr, "TRACE: ");

  void *parser = ParseAlloc(GC_malloc);
  token_iterate(tokens, ^(token_t *token) {
    Parse(parser, token->type, token, NULL);
  });

  GList *ast = NULL;
  Parse(parser, 0, NULL, (void **) &ast);

  char *xml = g_strdup_printf("%s.xml", argv[ 1 ]);

  printf("emitting tree to %s\n", xml);
  FILE *fp = fopen(xml, "wb");
  char *xmlast = pp_ast(ast);
  fwrite(xmlast, sizeof(char), strlen(xmlast), fp);
  fclose(fp);

  g_free(xmlast);
  g_free(xml);
  g_free(data);
  ParseFree(parser, GC_free);
  tokens_free(tokens);
  g_list_free_full(ast, (GDestroyNotify) ast_free);

  return 0;
}
