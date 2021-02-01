#include <pthread.h>

#include <iostream>

void *Work(void *) {
  std::cout << pthread_self() << ": Work! work! work!\n";
  return nullptr;
}

void *JoinAll(void *thread_ids) {
  pthread_t *tids = reinterpret_cast<pthread_t *>(thread_ids);

  puts("");
  std::cout << "Joiner thread id: " << pthread_self() << std::endl;
  for (int i = 0; i < 8; i++) {
    std::cout << "Thread with id " << tids[i] << " is going to join.\n";
    if (!pthread_equal(tids[i], pthread_self())) pthread_join(tids[i], nullptr);
  }
  puts("");

  return nullptr;
}

int main() {
  std::cout << "Main thread id: " << pthread_self() << std::endl;
  puts("");

  pthread_t thread_ids[8];

  // 子线程
  for (int i = 0; i < 7; i++)
    pthread_create(&thread_ids[i], nullptr, &Work, nullptr);

  pthread_create(&thread_ids[7], nullptr, &JoinAll, &thread_ids);

  pthread_join(thread_ids[7], nullptr);
  std::cout << "Joiner thread id: " << pthread_self() << std::endl;
  std::cout << "Thread with id " << thread_ids[7] << " is going to join.\n";
  puts("");

  return 0;
}