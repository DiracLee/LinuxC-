#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>

sig_atomic_t child_exit_status;

extern "C" {
void Cleanup(int sig_num) {
  int status;
  wait(&status);
  child_exit_status = status;
}
}

int main() {
  struct sigaction sa;
  memset(&sa, 0, sizeof sa);
  sa.sa_handler = &Cleanup;
  sigaction(SIGCHLD, &sa, nullptr);

  std::cout << "Main process PID: " << getpid() << std::endl;

  pid_t child_pid = fork();

  if (child_pid != 0) {
    std::cout << "In parent process with PID: " << getpid() << std::endl;
    std::cout << "My child PID: " << child_pid << std::endl;
  } else {
    std::cout << "In child process with PID: " << getpid() << std::endl;
  }

  return 0;
}