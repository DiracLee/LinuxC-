#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

std::mutex x;
int r;  // critical resource

class Worker {
 public:
  explicit Worker(int no, int a, int b) : no_(no), a_(a), b_(b) {}

  void work(int *r) {
    {
      std::lock_guard<std::mutex> locker(x);
      *r = a_ + b_;
    }
    std::cout << "Thread No.: " << no_ << std::endl;
    std::cout << a_ << " + " << b_ << " = " << a_ + b_ << std::endl;
  }

 private:
  int no_, a_, b_, r_;
};

int main() {
  std::vector<Worker *> workers(8);
  std::vector<std::thread *> threads(8);

  for (int i = 0; i < 8; i++) {
    workers[i] = new Worker(i, i - 1, i + 1);
    threads[i] = new std::thread(
        std::bind(&Worker::work, workers[i], std::placeholders::_1), &r);
  }

  for (int i = 0; i < 8; i++) threads[i]->join();

  return 0;
}