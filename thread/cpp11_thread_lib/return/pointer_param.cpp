#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <tuple>

std::mutex x;  // lock retrun pointer param

class Worker {
 public:
  Worker(int no, int a, int b) : no_(no), a_(a), b_(b) {}

  void work(int *r) {
    x.lock();
    *r = a_ + b_;
    x.unlock();
  }

 private:
  int no_;
  int a_, b_;
};

int main() {
  std::tuple<Worker *, int, std::thread *> workers[8];

  for (int i = 0; i < 8; i++)
    workers[i] = std::make_tuple(new Worker{i, i - 1, i + 1}, 0, nullptr);

  std::cout << "Before workers start to work:\n";
  for (int i = 0; i < 8; i++)
    std::cout << "No." << i << " worker: result =  " << std::get<1>(workers[i])
              << std::endl;

  std::cout << "Workers is ready to work:\n";
  for (int i = 0; i < 8; i++)
    std::get<2>(workers[i]) =
        new std::thread(std::bind(&Worker::work, std::get<0>(workers[i]),
                                  std::placeholders::_1),
                        &std::get<1>(workers[i]));

  for (int i = 0; i < 8; i++) {
    std::get<2>(workers[i])->join();
    delete std::get<0>(workers[i]);
    std::get<0>(workers[i]) = nullptr;
    delete std::get<2>(workers[i]);
    std::get<2>(workers[i]) = nullptr;
  }

  std::cout << "After workers finished their work:\n";
  for (int i = 0; i < 8; i++)
    std::cout << "No." << i << " worker: result =  " << std::get<1>(workers[i])
              << std::endl;

  return 0;
}