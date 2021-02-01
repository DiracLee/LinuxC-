#include <pthread.h>

#include <iostream>

void *Print1s(void *) {
  static int count1 = 1000;
  while (count1--) {
    std::cerr << "1";
  }
  return nullptr;
}

void *Print2s(void *) {
  static int count2 = 1000;
  while (count2--) {
    std::cerr << "2";
  }
  return nullptr;
}

int main() {
  pthread_t thread_id;
  pthread_attr_t *attr = nullptr;
  void *args = nullptr;

  // 子线程
  pthread_create(&thread_id, attr, &Print1s, args);

  // 主线程
  Print2s(nullptr);

  return 0;
}