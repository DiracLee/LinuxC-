#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

std::mutex x;

void Func() {
  x.lock();

  std::cout << std::this_thread::get_id() << "is entering..." << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << std::this_thread::get_id() << "is leaving..." << std::endl;

  x.unlock();
}

int main() {
  std::vector<std::thread*> threads(8);
  for (int i = 0; i < 8; i++) threads[i] = new std::thread(&Func);

  for (int i = 0; i < 8; i++) threads[i]->join();
  return 0;
}