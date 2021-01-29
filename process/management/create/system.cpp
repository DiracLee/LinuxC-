#include <iostream>

int main()
{
  const char* cmd = "pwd";
  std::cout << "Run command: " << cmd << std::endl;
  int ret = system("pwd");
  std::cout << "Return value: " << ret << std::endl;
  return 0;
}