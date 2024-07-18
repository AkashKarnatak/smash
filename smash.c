#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

char *trim(char *buf, ssize_t *n) {
  size_t len = *n;
  for (size_t i = 0; i < len && isspace(*buf); ++i, --*n)
    ++buf;
  for (; *n > 0 && isspace(buf[*n - 1]); --*n)
    buf[*n - 1] = '\0';
  return buf;
}

size_t split(char *buf, size_t n, char ***tokens_h) {
  if (n == 0) {
    return 0;
  }

  size_t capacity = 4;
  char **args = (char **)malloc(capacity * sizeof(char *));
  size_t size = 0;

  int prev_space = 1;
  for (size_t i = 0; i < n; ++i, ++buf) {
    int curr_space = isspace(*buf);
    if (!prev_space && curr_space) {
      *buf = '\0';
    } else if (prev_space && !curr_space) {
      if (size + 1 == capacity) {
        size_t new_capacity = 2 * capacity;
        char **new_args = (char **)realloc(args, new_capacity * sizeof(char *));
        if (!new_args) {
          free(args);
          *tokens_h = NULL;
          return -1;
        }

        capacity = new_capacity;
        args = new_args;
      }
      args[size++] = buf;
    }
    prev_space = curr_space;
  }

  args[size++] = NULL;
  *tokens_h = args;
  return size;
}

int main() {
  // stores user input
  char *line_h = NULL;
  size_t size = 0;

  // REPL - read, eval, print loop
  for (;;) {
    // prompt string
    fprintf(stderr, "$ ");
    // read input from user
    ssize_t len = getline(&line_h, &size, stdin);

    if (len == -1 && errno == 0) { // EOF
      fprintf(stderr, "\nexit\n");
      free(line_h);
      return 0;
    } else if (len == -1) { // error reading file
      perror("smash: getline()");
      free(line_h);
      continue;
    }

    // trim input
    char *trimmed_line = trim(line_h, &len);

    // tokenize string
    char **tokens_h = NULL;
    size_t n_tokens = split(trimmed_line, len, &tokens_h);
    if (n_tokens == -1) {
      fprintf(stderr, "smash: tokenization failed\n");
      continue;
    }

    if (n_tokens == 0)
      continue;

    if (strncmp(tokens_h[0], "exit", 4) == 0) {
      fprintf(stderr, "exit\n");
      free(line_h);
      return 0;
    } else {
      pid_t child_pid = fork();

      if (child_pid == -1) {
        perror("smash: fork() failed");
        continue;
      }

      // parent process
      if (child_pid > 0) {
        if (wait(NULL) == -1) {
          perror("smash: wait() failed");
        }
        continue;
      }

      // child process
      if (execvp(tokens_h[0], tokens_h) == -1) {
        perror("smash: exec() failed");
        exit(1);
      }
    }

    free(tokens_h);
  }
  free(line_h);
  return 0;
}
