#include <pthread.h>

#include <iostream>

void *Work(void *) {
  std::cout << pthread_self() << ": Work! work! work!\n";
  return nullptr;
}

int main() {
  pthread_t thread_id;
  pthread_attr_t attr;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  pthread_create(&thread_id, &attr, &Work, nullptr);

  pthread_attr_destroy(&attr);

  return 0;
}