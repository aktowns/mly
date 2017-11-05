#include <stdlib.h>
#include <stdint.h>
#include <linenoise.h>

void repl() {
  char *line;
  while ((line = linenoise("> ")) != NULL) {

    free(line);
  }
}