#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>

int spawn(char *program, char **args) {
  pid_t child_pid = fork();

  if (child_pid != 0) {
    return child_pid;
  } else {
    execvp(program, args);

    std::cerr << "Error occurred when executing execvp" << std::endl;
    abort();
  }
}

int main() {
  char *args[] = {"ls", "-l", "/", nullptr};
  spawn("ls", args);
  std::cout << "Done!" << std::endl;
  return 0;
}