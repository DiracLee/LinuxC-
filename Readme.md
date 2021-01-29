# Linux C++

## 系统调用

### 程序执行环境

#### 参数列表

传给 `main` 函数的参数 `agrc` 和 `argv` 

- `agrc` 为命令参数总个数

- `argv[0]` 为所执行的命令文件名


参数解析需要的辅助工具

- 结构体 `option`：定义在头文件 `getopt.h` 中，用于自定义参数解析规则。

  ```c
  struct option
  {
      const char* name;  // 选项长名称  --name
      int has_arg;       // 该选项是否具有附加参数(0: 无; 1: 有; 2: 可选)
      int* flag;         // 用于保存val的值，设为 nullptr
      int val;           // 选项短名称，若输入为长选项，则此处保存对应对端名称
  };
  ```

- 函数 `getopt_long()`，用于解析用户传入的参数，函数返回值为参数短名称, 参数无效时返回 '?', 不存在时返回 -1

  ```c
  int getopt_long(int argc, char* argv[], 
                  const char* short_options,          // 可匹配的短选项字符列表，有附加参数则在后面添加 ':'
                  const struct option* long_options,  // option 数组，指示解析规则
                  int* long_index);                   // 所解析出的长选项在 option 数组中的索引下标
  ```



- 全局变量 `optarg`，函数 `getopt_long()` 调用时自动将附加参数存储到 `optarg` 中

编写程序，接受以下三个选项并执行正确操作

```
-h           --help                 # 显式程序用法并退出
-o filename  --output filename      # 指定输出文件名
-v           --verbose              # 输出复杂信息
```

程序代码

```c++
#include <getopt.h>

#include <cstdlib>
#include <iostream>

using namespace std;

const char *program_name;

const char *output_filename;
int verbose = 0;

void OutputInfo(ostream &os, int exit_code) {
  cout << "Usage: " << program_name << "<options> [file]" << endl;
  cout << "-h, --help: Display this usage infomation." << endl;
  cout << "-o, --output <file>: Write the output to file." << endl;
  cout << "-v, --verbose: Print verbose messages." << endl;
  exit(exit_code);
}

int main(int argc, char *argv[]) {
  program_name = argv[0];

  const char *const shot_opts =
      "ho:v";  // shot option "-o" has additional params.

  const struct option long_opts[] = {{"help", 0, nullptr, 'h'},
                                     {"output", 1, nullptr, 'o'},
                                     {"verbose", 0, nullptr, 'v'},
                                     {nullptr, 0, nullptr, 0}};
  // return shot option name
  //        '?' when invalid name
  //        -1  when no option given
  int opt = getopt_long(argc, argv, shot_opts, long_opts, nullptr);

  while (opt != -1) {
    switch (opt) {
      case 'h':
        OutputInfo(cout, 0);
      case 'o': {
        output_filename = optarg;  // additional params are stored in `optarg`
        cout << "The output filename is set as: " << output_filename << endl;
        break;
      }
      case 'v': {
        verbose = 1;
      }
      case '?': {
        OutputInfo(cout, 1);
      }
      case -1: {
        break;
      }
      default: {
        abort();
      }
    }

    opt = getopt_long(argc, argv, shot_opts, long_opts, nullptr);
  }

  return 0;
}
```


#### 环境变量

典型的Linux环境变量

-   `USER`：当前用户名
-   `HOME`：当前用户主目录
-   `PATH`：冒号分隔的Linux查找命令目录列表

shell处理

-   查看环境变量，如：`echo $USER`
-   设置新的环境变量
    -   `EDITOR=emacs;export EDITOR`
    -   `export EDITOR=emacs`

环境变量内部定义格式：`VARIABLE=value`

使用`getenv()`函数返回环境变量的值

使用全局变量 `environ` (一个字符串数组) 处理环境变量

```c++
#include <iostream>
#include <cstdlib>

using namespace std;

extern char** environ;

int main()
{
    char* server_name = getenv("SERVER_NAME");
    if (!server_name)                        // 用户未指定环境变量SERVER_NAME
        server_name = "server.example.com";  // 使用默认值
    
    cout << "accessing server " << server_name << endl;
    return 0;
}
```

#### 程序退出码

程序退出码：结束时传递给操作系统的整型数据

-   实际上是`main()`函数的返回值
-   其他函数也可以调用`exit()`函数返回特定退出码
-   退出码的变量名称经常使用`exit_code`
-   应该仔细设计程序退出码，确保它们能够区分不同错误

操作系统相应程序退出码，如果必要，执行后续处理

-   shell编程时查看上一次退出码指令：`echo $?`


---

## 进程编程

进程
- 描述程序执行与资源共享的基本单位
- 创建进程：`system()`、`fork()`、`exec()`
- 终止进程：`kill()`
- 等待进程终止：`wait()`、`waitpid()`

进程组
- 由多个相关进程组成，可以将信号发送给进程组内所有进程，从而协调它们的行为
- 获取进程组 ID：`pid_t getpgid(pid_t pid);`
- 设置进程组 ID：`int setpgid(pid_t pid, pid_t pgid);`