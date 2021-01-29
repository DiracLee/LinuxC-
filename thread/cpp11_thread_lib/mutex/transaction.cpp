#include <iostream>
#include <mutex>
#include <thread>

class Account {
  friend void Transfer(Account &from, Account &to, double amount);

 public:
  explicit Account(double balance) : balance_(balance) {}

  double balance() { return balance_; }

  void Earn(double amount) { balance_ += amount; }

  void Pay(double amount) { balance_ -= amount; }

 private:
  double balance_;
  std::mutex x_;
};

void Transfer(Account &from, Account &to, double amount) {
  std::unique_lock<std::mutex> locker1(from.x_, std::adopt_lock);
  std::unique_lock<std::mutex> locker2(to.x_, std::adopt_lock);
  std::lock(from.x_, to.x_);
  from.Pay(amount);
  to.Earn(amount);
}

int main() {
  Account a1(100.0), a2(200.0);
  std::thread t1(&Transfer, std::ref(a1), std::ref(a2), 10.0);
  std::thread t2(&Transfer, std::ref(a2), std::ref(a1), 20.0);

  t1.join();
  t2.join();

  return 0;
}