#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>

union semun {
  int val;
  struct semid_ds* buf;
  unsigned short int* array;
  struct seminfo* __buf;
};

class Semaphore {
 public:
  Semaphore() : nsems(1), semflg(SEM_UNDO) {
    semid = semget(IPC_PRIVATE, nsems, semflg);
    ops = new struct sembuf[nsems];
  }
  Semaphore(key_t key, int nsems, int semflg) : nsems(nsems), semflg(semflg) {
    semid = semget(key, nsems, semflg);
    ops = new struct sembuf[nsems];
  }

  int Configure(unsigned short int values[]) {
    constexpr int SEMNUM = 0;
    union semun config = {.array = values};
    return semctl(semid, SEMNUM, SETALL, config);
  }

  int Clear() {
    constexpr int SEMNUM_IGNORED = 0;
    union semun ignored;
    return semctl(semid, SEMNUM_IGNORED, IPC_RMID, ignored);
  }

  int Wait(int op_num, int sem_num) {
    ops[op_num].sem_num = sem_num;
    ops[op_num].sem_op = -1;
    ops[op_num].sem_flg = semflg;
    return semop(semid, ops, nsems);
  }

  int Post(int op_num, int sem_num) {
    ops[op_num].sem_num = sem_num;
    ops[op_num].sem_op = 1;
    ops[op_num].sem_flg = semflg;
    return semop(semid, ops, nsems);
  }

 private:
  int semid;
  const int nsems;
  int semflg;
  struct sembuf* ops;
};

int main() {
  Semaphore sem(IPC_PRIVATE, 1, SEM_UNDO);

  unsigned short int values[1] = {0};
  sem.Configure(values);

  int child_pid = fork();

  if (child_pid != 0) {  // Parent process
    std::cout << "Parent: Need to feed.\n";
    sem.Wait(0, 0);
    std::cout << "Parent: Satisfied!\n";

    waitpid(child_pid, nullptr, 0);
  } else {  // Child process
    std::cout << "Child: Prepare...\n";
    sleep(5);
    std::cout << "Child: Ready!\n";
    sem.Post(0, 0);
  }

  sem.Clear();

  return 0;
}