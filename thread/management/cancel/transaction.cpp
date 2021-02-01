#include <pthread.h>

#include <iostream>
#include <tuple>

class Account {
 public:
  Account(double balance) : balance_(balance) {}

  double balance() const { return balance_; }

  void Earn(double amount) { balance_ += amount; }

  void Pay(double amount) { balance_ -= amount; }

 private:
  double balance_;
};

void Transfer(Account *from, Account *to, double amount) {
  int old_cancel_state;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_cancel_state);

  from->Pay(amount);
  to->Earn(amount);

  pthread_setcancelstate(old_cancel_state, nullptr);
}

void *PTransfer(void *data) {
  Account *a1, *a2;
  double amount;
  std::tie(a1, a2, amount) =
      *reinterpret_cast<std::tuple<Account *, Account *, double> *>(data);

  Transfer(a1, a2, amount);

  return nullptr;
}

int main() {
  Account a1(100.0), a2(200.0);
  pthread_t tid1, tid2;

  std::tuple<Account *, Account *, double> transaction1{&a1, &a2, 10.0};
  pthread_create(&tid1, nullptr, &PTransfer,
                 reinterpret_cast<void *>(&transaction1));

  std::tuple<Account *, Account *, double> transaction2{&a2, &a1, 20.0};
  pthread_create(&tid2, nullptr, &PTransfer,
                 reinterpret_cast<void *>(&transaction2));

  pthread_join(tid1, nullptr);
  pthread_join(tid2, nullptr);

  std::cout << "After the transactions,\n";
  std::cout << "\ta1's balance: " << a1.balance() << std::endl;
  std::cout << "\ta2's balance: " << a2.balance() << std::endl;

  return 0;
}