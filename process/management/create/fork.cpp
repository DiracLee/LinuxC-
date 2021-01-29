#include <sys/types.h>
#include <unistd.h>

#include <iostream>

int main() {
  std::cout << "The main process ID is" << getpid() << std::endl;

  pid_t child_pid = fork();

  if (child_pid != 0) {
    std::cout << "In parent process with PID " << getpid() << std::endl;
    std::cout << "The child process PID is " << child_pid << std::endl;
  }
  else {
    std::cout << "In child process with PID " << getpid() << std::endl;
  }

  return 0;
}