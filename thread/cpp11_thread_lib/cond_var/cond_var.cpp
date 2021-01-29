#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

std::mutex x;
std::condition_variable cond;
bool ready;

void Run(int no) {
  std::unique_lock<std::mutex> locker(x);

  cond.wait(locker, []() -> bool { return ready; });

  std::cout << "thread " << no << "active!\n";
}

int main() {
  std::thread threads[8];
  for (int i = 0; i < 8; i ++) threads[i] = std::thread(Run, i);
  std::cout << "8 threads ready.\n";
  {
    std::unique_lock<std::mutex> locker(x);
    ready = true;
    cond.notify_all();
  }

  for (int i = 0; i < 8; i ++) threads[i].join();

  return 0;
}