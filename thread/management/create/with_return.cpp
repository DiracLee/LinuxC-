#include <pthread.h>

#include <iostream>

void *IsPrime(void *n) {
  unsigned int num = reinterpret_cast<unsigned int>(n);  // needs `-fpermissive` compile arg
  if (num < 2) return reinterpret_cast<void *>(false);
  for (unsigned int x = 2; x <= num / x; x++) {
    if (num % x == 0) return reinterpret_cast<void *>(false);
  }
  return reinterpret_cast<void *>(true);
}

int main() {
  pthread_t thread_ids[8];
  bool results[8];

  for (int i = 0; i < 8; i++)
    pthread_create(&thread_ids[i], nullptr, &IsPrime,
                   reinterpret_cast<void *>(i * i + 1));

  for (int i = 0; i < 8; i++)
    pthread_join(thread_ids[i], reinterpret_cast<void **>(&results[i]));

  for (int i = 0; i < 8; i++)
    std::cout << i * i + 1 << (results[i] ? " is " : " is not ")
              << "a prime.\n";

  return 0;
}