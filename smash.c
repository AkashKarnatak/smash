#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *ltrim(char *buf, size_t n) {
  for (size_t i = 0; i < n && isspace(*buf); ++i)
    ++buf;
  return buf;
}

int main() {
  // stores user input
  char *line_h = NULL;
  size_t size = 0;

  // REPL - read, eval, print loop
  for (;;) {
    // prompt string
    printf("$ ");
    // read input from user
    ssize_t len = getline(&line_h, &size, stdin);

    if (len == -1 && errno == 0) { // EOF
      printf("\nexit\n");
      free(line_h);
      return 0;
    } else if (len == -1) { // error reading file
      fprintf(stderr, "Failed to read input\n");
      free(line_h);
      return -1;
    }

    // trim input
    char *trimmed_line = ltrim(line_h, len);

    // mirror the input
    printf("%s", trimmed_line);
  }
  free(line_h);
  return 0;
}
