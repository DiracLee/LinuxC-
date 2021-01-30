#include <atomic>
#include <iostream>
#include <thread>

std::atomic<int> a{0};
int n = 0;

void Add(int m) {
  while (m--) ++n;
}

void AddAtomically(int m) {
  while (m--) ++a;
}

int main() {
  std::thread ts1[32], ts2[32];

  for (auto &t : ts1) t = std::move(std::thread{AddAtomically, 10000});
  for (auto &t : ts2) t = std::move(std::thread{Add, 10000});

  for (auto &t : ts1) t.join();
  for (auto &t : ts2) t.join();

  std::cout << "a = " << a << std::endl;
  std::cout << "n = " << n << std::endl;

  return 0;
}