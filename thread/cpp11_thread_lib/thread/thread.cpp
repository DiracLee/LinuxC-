#include <functional>
#include <iostream>
#include <thread>

void Func0() {
  std::cout << "In Func0" << std::endl;
  std::cout << "Thread ID: " << std::this_thread::get_id() << std::endl;
}

void Func2(int a, int b) {
  std::cout << "In Func2" << std::endl;
  std::cout << "Thread ID: " << std::this_thread::get_id() << std::endl;
  std::cout << a << " + " << b << " = " << a + b << std::endl;
}

class Functor {
 public:
  void operator()(int a, int b) {
    std::cout << "In Functor" << std::endl;
    std::cout << "Thread ID: " << std::this_thread::get_id() << std::endl;
    std::cout << a << " + " << b << " = " << a + b << std::endl;
  }
};

class Worker {
 public:
  Worker() {}
  Worker(int a, int b) : a_(a), b_(b) {}

  void Work() {
    std::cout << "A worker is working ..." << std::endl;
    std::cout << "Thread ID: " << std::this_thread::get_id() << std::endl;
    std::cout << a_ << " + " << b_ << " = " << a_ + b_ << std::endl;
  }

 private:
  int a_{10}, b_{20};
};

int main() {
  // 0 params function
  std::thread t0(&Func0);

  // 2 params function
  int m = 10, n = 20;
  std::thread t2(&Func2, m, n);

  // 2 params functor
  std::thread tf(Functor(), m, n);

  // 0 params method
  std::thread tm(std::bind(&Worker::Work, Worker()));
  
  t0.join();
  t2.join();
  tf.join();
  tm.join();

  return 0;
}