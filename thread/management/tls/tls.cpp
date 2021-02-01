#include <pthread.h>

#include <iostream>

static pthread_key_t tlk;

void WriteToThreadLog(const char *msg) {
  FILE *fp = reinterpret_cast<FILE *>(pthread_getspecific(tlk));
  fprintf(fp, "Thread%d: %s\n", static_cast<int>(pthread_self()), msg);
}

void  CloseThreadLog( void * fp )
{
  fclose( ( FILE * )fp );
}

void *Work(void *) {
  char filename[256];
  sprintf(filename, "logs/thread%d.log", static_cast<int>(pthread_self()));

  FILE *fp = fopen(filename, "w");

  pthread_setspecific(tlk, fp);

  WriteToThreadLog("Thread starting...");

  return nullptr;
}

int main() {
  pthread_t tids[8];
  pthread_key_create(&tlk, &CloseThreadLog);

  for (int i = 0; i < 7; i++) pthread_create(&tids[i], nullptr, &Work, nullptr);

  for (int i = 0; i < 7; i++) pthread_join(tids[i], nullptr);

  pthread_key_delete(tlk);

  return 0;
}