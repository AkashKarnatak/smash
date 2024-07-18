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
    buf[*n-1] = '\0';
  return buf;
}

char **split(char *buf, size_t n, size_t *len) {
  if (n == 0) {
    *len = 0;
    return NULL;
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
      if (size == capacity) {
        size_t new_capacity = 2 * capacity;
        char **new_args = (char **)realloc(args, new_capacity * sizeof(char *));
        if (!new_args) {
          free(args);
          *len = 0;
          return NULL;
        }

        capacity = new_capacity;
        args = new_args;
      }
      args[size++] = buf;
    }
    prev_space = curr_space;
  }

  *len = size;
  return args;
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
    char *trimmed_line = trim(line_h, &len);

    // tokenize string
    size_t n_tokens = 0;
    char **tokens_h = split(trimmed_line, len, &n_tokens);
    char *args[n_tokens + 1];
    for (size_t i = 0; i < n_tokens; ++i) {
      args[i] = tokens_h[i];
    }
    args[n_tokens] = NULL;

    if (n_tokens == 0) continue;

    if (strncmp(args[0], "exit", 4) == 0) {
      printf("exit\n");
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
      if (execvp(args[0], args) == -1) {
        perror("smash: exec() failed");
        exit(1);
      }
    }

    free(tokens_h);
  }
  free(line_h);
  return 0;
}
