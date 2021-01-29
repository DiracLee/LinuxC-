#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

sig_atomic_t sigusr1_count = 0;

extern "C" {
void OnSigUsr1(int signal_number) { ++sigusr1_count; }
}

int main() {
  std::cout << "Type in shell: `kill -s SIGUSR1 " << getpid()
            << "` to pass me signal `SIGUSR1` and kill this process" << std::endl;
  // kill -s SIGUSR1 <pid>

  struct sigaction sa;
  memset(&sa, 0, sizeof sa);

  sa.sa_handler = &OnSigUsr1;

  sigaction(SIGUSR1, &sa, nullptr);

  sleep(100);

  std::cout << "SIGUSR1 counts: " << sigusr1_count << std::endl;

  return 0;
}