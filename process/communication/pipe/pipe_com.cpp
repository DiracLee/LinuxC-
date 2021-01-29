#include <stdlib.h>
#include <unistd.h>

#include <iostream>

const int buf_size = getpagesize();

void Read(FILE* stream) {
  char buf[buf_size];

  while (!feof(stream) && !ferror(stream) &&
         fgets(buf, sizeof buf, stream) != nullptr) {
    fprintf(stdout, "Data received: \n");
    fputs(buf, stdout);
  }
}

void Write(const char* msg, int count, FILE* stream) {
  while (count--) {
    fprintf(stream, "%s\n", msg);
    fflush(stream);
    sleep(1);
  }
}

int main() {
  int fds[2];  // 0: read  1: write
  pipe(fds);

  pid_t child_pid = fork();

  if (child_pid != 0) {  // Parent process: Read
    close(fds[1]);

    FILE* stream = fdopen(fds[0], "r");
    Read(stream);

    close(fds[0]);
  } else {
    close(fds[0]);

    char buf[buf_size];
    for (int i = 0; i < buf_size - 2; i++) buf[i] = 'A' + i % 26;
    buf[buf_size - 1] = buf[buf_size - 2] = '\0';

    FILE* stream = fdopen(fds[1], "w");
    Write(buf, 3, stream);

    close(fds[1]);
  }

  return 0;
}