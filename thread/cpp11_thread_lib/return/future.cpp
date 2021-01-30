#include <exception>
#include <future>
#include <iostream>
#include <thread>
#include <tuple>

class Worker {
 public:
  Worker(int no, int a, int b) : no_(no), a_(a), b_(b) {}

  int work() {
    if (a_ < -10 || b_ < -10)
      throw std::range_error("Won't happen.");
    return a_ + b_;
  }

 private:
  int no_;
  int a_, b_;
};

int main() {
  std::tuple<Worker*, std::future<int>> workers[8];

  for (int i = 0; i < 8; i++)
    workers[i] = std::make_tuple(new Worker{i, i - 1, i + 1}, std::future<int>{});

  std::cout << "Before workers start to work:\n";
  for (int i = 0; i < 8; i++)
    std::cout << "No." << i
              << " worker: result =  " << 0
              << std::endl;

  std::cout << "Workers is ready to work:\n";

  for (int i = 0; i < 8; i++)
    std::get<1>(workers[i]) =
        std::move(std::async(&Worker::work, std::get<0>(workers[i])));

  std::cout << "After workers finished their work:\n";
  try {
    for (int i = 0; i < 8; i++) {
      std::cout << "No." << i
                << " worker: result =  " << std::get<1>(workers[i]).get()
                << std::endl;
    }
  } catch (const std::range_error& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}