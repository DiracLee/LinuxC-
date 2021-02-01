#include <pthread.h>

#include <iostream>

class Worker {
 public:
  explicit Worker(int no, int a, int b) : no_(no), a_(a), b_(b) {}

  void Work() const {
    std::cout << "In No." << no_ << " thread:\n"
              << a_ << " + " << b_ << " = " << a_ + b_ << std::endl;
  }

 private:
  int no_;
  int a_, b_;
};

void *Employ(void *worker) {
  Worker *w = reinterpret_cast<Worker *>(worker);
  w->Work();
  return nullptr;
}

struct PWorker {
  pthread_t thread_id;
  Worker *worker;
};

int main() {
  pthread_t thread_ids[8];
  Worker *workers[8];

  for (int i = 0; i < 8; i++) workers[i] = new Worker{i, i - 1, i + 1};

  for (int i = 0; i < 8; i++)
    pthread_create(&thread_ids[i], nullptr, &Employ,
                   reinterpret_cast<void *>(workers[i]));

  for (int i = 0; i < 8; i++) pthread_join(thread_ids[i], nullptr);

  return 0;
}