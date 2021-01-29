#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>

const int buf_size = getpagesize();

int main() {
  int fds[2];
  pipe(fds);

  pid_t child_pid = fork();

  if (child_pid != 0) {  // Parent process: Read
    close(fds[1]);

    char buf[buf_size];
    FILE *stream = fdopen(fds[0], "r");

    fprintf(stdout, "%s\n", buf);
    while (!feof(stream) && !ferror(stream) &&
           fgets(buf, sizeof buf, stream) != nullptr) {
      fputs(buf, stdout);
    }

    close(fds[0]);

    waitpid(child_pid, nullptr, 0);
  } else {  // Child process: Write, dup from stdout, i.e., STDOUT_FILENO out and then fds[1] in
    close(fds[0]);

    dup2(fds[1], STDOUT_FILENO);
    char *args[] = { "ls", "-l", "/", nullptr };
    execvp(args[0], args);

    close(fds[1]);
  }

  return 0;
}