#include <iostream>

using namespace std;

extern char** environ;  // 指针数组，每个元素都指向一个环境变量字符串的首地址

int main() {
  char** var;
  for (var = environ; *var != nullptr; ++var) {
    cout << *var << endl;
  }

  char* current_user = getenv("USER");
  cout << current_user << endl;
  return 0;
}