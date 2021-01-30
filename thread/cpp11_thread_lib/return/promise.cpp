#include <exception>
#include <functional>
#include <future>
#include <iostream>
#include <thread>
#include <tuple>

class Worker {
 public:
  Worker(int no, int a, int b) : no_(no), a_(a), b_(b) {}

  int work() { return a_ + b_; }

  void DoWork(std::promise<int> &&p) {
    try {
      p.set_value(work());
    } catch (...) {
      p.set_exception(std::current_exception());
    }
  }

 private:
  int no_;
  int a_, b_;
};

int main() {
  std::promise<int> ps[8];
  std::tuple<Worker *, std::future<int>> workers[8];
  for (int i = 0; i < 8; i++) {
    std::get<0>(workers[i]) = new Worker{i, i - 1, i + 1};
    std::get<1>(workers[i]) = ps[i].get_future();
  }

  for (int i = 0; i < 8; i++) {
    std::thread t{std::bind(&Worker::DoWork, std::get<0>(workers[i]),
                            std::placeholders::_1),
                  std::move(ps[i])};

    t.detach();
  }

  try {
    for (int i = 0; i < 8; i++) {
      std::cout << "No." << i
                << " worker: result =  " << std::get<1>(workers[i]).get()
                << std::endl;
    }
  } catch (std::range_error e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}