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

- `USER`：当前用户名
- `HOME`：当前用户主目录
- `PATH`：冒号分隔的Linux查找命令目录列表

shell处理

- 查看环境变量，如：`echo $USER`
- 设置新的环境变量
  - `EDITOR=emacs;export EDITOR`
  - `export EDITOR=emacs`

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

- 实际上是`main()`函数的返回值
- 其他函数也可以调用`exit()`函数返回特定退出码
- 退出码的变量名称经常使用`exit_code`
- 应该仔细设计程序退出码，确保它们能够区分不同错误

操作系统相应程序退出码，如果必要，执行后续处理

- shell编程时查看上一次退出码指令：`echo $?`

------

## 进程编程

### 进程的基本概念

进程

- 描述程序执行与资源共享的基本单位
- 创建进程：`system()`、`fork()`、`exec()`
- 终止进程：`kill()`
- 等待进程终止：`wait()`、`waitpid()`

进程组

- 多个相关进程的集合。可以将信号发送给进程组内所有进程，从而协调它们的行为
- 获取进程组 ID：`pid_t getpgid(pid_t pid);`
  - `pid`：传入 0 时表示获取当前进程的组 ID
  - 返回值：进程的组 ID，出错时返回 -1，并设置 `errno` 值
- 设置进程组 ID：`int setpgid(pid_t pid, pid_t pgid);`
  - `pid`：传入 0 时表示设置当前进程的进程组 ID
  - `pgid`：传入 0 时表示将改进程的组ID设置为当前进程的组 ID

会话（Session）

- 多个相关进程组的集合，记录登录用户的全部活动
- 登录 Shell 进程成为会话首领，其 PID 设置为会话 ID
- 非绘画首领进程通过调用 `setsid()` 函数创建新会话，成为首领
- 获取会话 ID：`pid_t getsid(pid_t pid)`
  - 传入 0 时表示获取当前进程的会话 ID
  - 返回值：进程的会话 ID，出错时返回 -1，并设置 `errno` 值
- 创建会话：`pid_t setsid()`
  - 返回值：新创建的会话 ID，出错时返回 -1，并设置 `errno` 值

### 信号

信号是发送给进程的特殊异步消息，当进程接收到信号时立即处理，无需完成当前代码行。

常用信号

- `SIGTERM`：请求进程终止
- `SIGKILL`：强制进程终止
- `SIGUSR1`、`SIGUSR2`：用户自定义信号

`struct sigaction`：设置信号的行为，通过配置成员 `sa_handler` 有三种取值来实现

- `SIG_DFL`：使用系统提供的该信号的默认配置
- `SIG_IGN`：忽略该信号
- `void (*)(int signal_number)` 函数指针：按照该函数来处理信号

注意事项：

- 信号处理是异步操作，处理信号时主程序非常脆弱
- 信号处理例程应当尽可能小，甚至可能被新信号所中断
- 不要在信号处理例程中进行 I/O 操作、系统函数、库函数
- 不要在信号处理例程中进行复杂的赋值操作，非原子操作可能在执行时被打断
- 使用 `sig_atomic_t` 类型的全局变量进行赋值

```c++
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

sig_atomic_t sigusr1_count = 0;

extern "C" {
void OnSigUsr1(int signal_number) { ++sigusr1_count; }
}

int main() {
  std::cout << "Type in shell: `kill -s SIGUSR1 " << getpid()
            << "` to pass me signal `SIGUSR1` and kill this process" << std::endl;
  // kill -s SIGUSR1 <pid>

  struct sigaction sa;
  memset(&sa, 0, sizeof sa);

  sa.sa_handler = &OnSigUsr1;

  sigaction(SIGUSR1, &sa, nullptr);

  sleep(100);

  std::cout << "SIGUSR1 counts: " << sigusr1_count << std::endl;

  return 0;
}
```



### 进程管理

#### 进程创建

`system()`：在程序中执行 Shell 命令

- 原型：`int system(const char* cmd);`
- 创建子进程运行 `cmd` 命令
- 返回 Shell 命令的退出状态码，无法运行返回 127，运行出错返回 -1

`fork()`：创建当前进程的副本作为子进程

- 原型：`pid_t fork();`
- 在子进程中返回 0，在父进程中返回子进程的 PID
- 通过判断返回值是否为 0 来确定当前代码是在哪个程序中执行

```c++
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

int main() {
  std::cout << "The main process ID is" << getpid() << std::endl;

  pid_t child_pid = fork();

  if (child_pid != 0) {
    std::cout << "In parent process with PID " << getpid() << std::endl;
    std::cout << "The child process PID is " << child_pid << std::endl;
  }
  else {
    std::cout << "In child process with PID " << getpid() << std::endl;
  }

  return 0;
}
```



#### 执行命令

`exec()` 函数簇

- `int execl(const char* path, const char* arg, ...);`
- `int execlp( const char * file, const char * arg, … );`
- `int execle( const char * path, const char * arg, …, char * const envp[] );`
- `int execv( const char * path, char * const argv[] );`
- `int execvp( const char * file, char * const argv[] );`
- `int execvpe( const char * file, char * const argv[], char * const envp[] );`

`exec()` 函数簇说明

- 包含字母 p：在当前执行路径中按程序名查找
- 包含字母 v：接受以 NULL 结尾的字符串数组格式的参数列表
- 包含字母 l：接受 C 格式的可变参数列表
- 包含字母 e：接受附加的环境参数列表，格式为以 NULL 结尾的字符串数组，且字符串格式为 `VARIABLE=value`

#### 进程调度

子进程与父进程的调度没有固定顺序，不能事先假定。

进程调度策略函数（头文件 `sched.h`）

- 获取进程调度策略：`int sched_getscheduler(pid_t pid);`
- 设置进程调度策略：`int sched_setscheduler(pid_t pid, int policy, const struct sched_param *sp);`
- 获取进程调度参数：`int sched_getparam(pid_t pid, struct sched_param *sp);`
- 设置进程调度参数：`int sched_setparam(pid_t pid, const struct sched_param *sp);`

进程优先级调整（头文件 `sys/time.h` 和 `sys/resource.h`

- 改变进程优先级：`int nice( int inc );`（头文件 `unistd.h`）
- 获取进程优先级：`int getpriority( int which, int who );`
- 设置进程优先级：`int setpriority( int which, int who, int prio );`

处理器亲和性（头文件 `sched.h`）

- 获取进程的处理器亲和性：`int sched_getaffinity( pid_t pid, size_t cpusetsize, cpu_set_t * mask );`
- 设置进程的处理器亲和性：`int sched_setaffinity( pid_t pid, size_t cpusetsize, cpu_set_t * mask );`

#### 进程终止

终止进程函数：`kill()`

- 头文件 `sys/types.h` 和 `signal.h`
- 原型： `int kill( pid_t pid, int sig );`
- 函数参数：`pid` 为子进程ID，`sig` 应为进程终止信号 `SIGTERM`

等待进程结束函数：`wait()`

- 原型：
  - `pid_t wait( int * status );` 
  - `pid_t waitpid( pid_t pid, int * status, int options );`
- 阻塞主调进程，直到一个子进程结束
- `WEXITSTATUS( int status )` 宏：查看子进程的退出码
- `WIFEXITED( int status )` 宏：确定子进程的退出状态是正常退出，还是未处理信号导致的意外死亡

```c++
#include <iostream>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>    //  必须包含此头文件，否则与wait共用体冲突
#include <unistd.h>

int spawn( char * program, char ** arg_list );

int main ()
{
  char * arg_list[] = { "ls", "-l", "/", NULL };
  spawn( "ls", arg_list );
  int child_status;
  wait( &child_status );    //  等待子进程结束
  if( WIFEXITED( child_status ) )    //  判断子进程是否正常退出
    cout << "Exited normally with " << WEXITSTATUS(child_status) <<endl;
  else
    cout << "Exited abnormally." << endl;
  cout << "Done!\n";
  return 0;
}
```



#### 僵尸进程

子进程已结束，但父进程未调用 `wait()` 函数等待

- 子进程已终止，但没有被正确清除，成为僵尸进程

清除子进程的手段

- 父进程调用 `wait()` 函数可确保子进程被清除
- 即使子进程在父进程调用 `wait()` 函数前已死亡（成为僵尸），其退出状态也可以被抽取出来，然后被清除
- 未清除的子进程自动被 init 进程收养

```c++
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
int main ()
{
  pid_t child_pid;
  child_pid = fork();
  if( child_pid > 0 )	//  父进程，速度睡眠六十秒
    sleep( 60 );
  else			//  子进程，立即退出
    exit( 0 );
  return 0;
}
```



#### 子进程异步清除

`SIGCHLD` 信号：子进程终止时，向父进程自动发送，编写此信号处理例程，异步清除子进程

```c++
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

sig_atomic_t child_exit_status;

extern "C"  {
void CleanUp( int sig_num )
  {
    int status;
    wait( &status );		//  清除子进程
    child_exit_status = status;	//  存储子进程的状态
  }
}

int main ()
{
  //  处理SIGCHLD信号
  struct sigaction sa;
  memset( &sa, 0, sizeof(sa) );
  sa.sa_handler = &CleanUp;
  sigaction( SIGCHLD, &sa, NULL );

  //  正常处理代码在此，例如调用fork()创建子进程
    
  //  子进程终止时会自动向父进程发送 SIGCHLD 信号，触发 Cleanup 例程执行

  return 0;
}
```



#### 守护进程

创建守护进程的步骤

- 创建新进程：新进程将成为未来的守护进程
- 守护进程的父进程退出：保证祖父进程确认父进程已结束，且守护进程不是组长进程
- 守护进程创建新进程组和新会话：并成为两者的首进程，此时刚创建的新会话还没有关联控制终端
- 改变工作目录：守护进程一般随系统启动，工作目录不应继续使用继承的工作目录
- 重设文件权限掩码：不需要继承文件权限掩码
- 关闭所有文件描述符：不需要继承任何打开的文件描述符
- 标准流重定向到 `/dev/null`

```c++
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fs.h>

int main()
{
  pid_t pid = fork();
  if( pid == -1 )		return -1;
  else if( pid != 0 )		exit( EXIT_SUCCESS );

 //  子进程
  if( setsid() == -1 )		return -2;
  //  设置工作目录
  if( chdir( "/" ) == -1 )	return -3;
  //  重设文件权限掩码
  umask( 0 );
  //  关闭文件描述符
  for( int i = 0; i < 3; i++ )	close( i );
  //  重定向标准流
  open( "/dev/null", O_RDWR );	// stdin
  dup( 0 );				// stdout
  dup( 0 );				// stderr
  //  守护进程的实际工作代码在此
  return 0;
}
```

守护进程创建函数 `daemon()`

- 实现了前述功能，减轻编写守护进程的负担
- 原型：`int daemon( int nochdir, int noclose );`
- 参数：若 `nochdir` 非 0，不更改工作目录；若 `noclose` 非 0，不关闭所有打开的文件描述符；一般均设为 0
- 返回值：成功时返回 0，失败时返回 -1，并设置 `errno` 值

### 进程通信

- 管道：相关进程间的顺序通信
- 进程信号量：进程间通信的同步控制机制
- 共享内存：允许多个进程读写同一片内存区域
- 映射内存：与共享内存意义相同，但与文件相关联
- 消息队列：在进程间传递二进制块数据
- 套接字：支持无关进程，甚至不同计算机进行通信

#### 管道

管道（pipe）的性质与意义

- 管道是允许单向通信的自动同步设备（半双工）
- 数据在写入端写入，在读取端读取
- 管道为串行设备，数据的读取顺序与写入顺序完全相同

管道的用途

- 只能用于有亲缘关系的进程，例如父进程和子进程之间通信

注意事项

- 管道的数据容量有限，一般为一个内存页面
- 如果写入速度超过读取速度，写入进程将阻塞，直到容量有空闲
- 如果读取速度超过写入速度，读取进程将阻塞，直到管道有数据

`pipe()` 函数：创建管道

- 头文件：`unistd.h` 和 `fcntl.h`
- 原型：`int pipe( int pipefd[2] ); `
- 参数：一个包含两个元素的整数数组，元素类型为文件描述符，0 号元为读取文件描述符，1 号元为写入文件描述符
- 返回值：成功时返回 0，不成功时返回 -1，并设置 `errno` 值

```c++
int pipe_fds[2];
int read_fd;
int write_fd;
pipe( pipe_fds );
read_fd = pipe_fds[0];
write_fd = pipe_fds[1];
```

##### 管道通信

```c++
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

const int buf_size = 4096;

//  向stream中写入count次msg
void Write( const char * msg, int count, FILE * stream )
{
  for( ; count > 0; --count )
  {
    fprintf( stream, "%s\n", msg );
    fflush( stream );
    sleep (1);
  }
}

//  从stream中读取数据
void Read( FILE * stream )
{
  char buf[buf_size];
  //  一直读取到流的尾部
  while( !feof(stream) && !ferror(stream) && fgets(buf, sizeof(buf), stream) != NULL )
  {
    fprintf( stdout, "Data received: \n" );
    fputs( buf, stdout );
  }
}

int main()
{
  int fds[2];
  pipe( fds );		//  创建管道
  pid_t pid = fork();	//  创建子进程
  if( pid == 0 )  {		//  子进程
    close( fds[1] );		//  只读取，关闭管道写入端
    //  将文件描述符转换为FILE *，以方便C/C++标准库函数处理
    FILE * stream = fdopen( fds[0], "r" );
    Read( stream );		//  从流中读取数据
    close( fds[0] );		//  关闭管道读取端
  }
  else if( pid > 0 )  {	//  父进程
    char buf[buf_size];	//  数据缓冲区，末尾封装两个‘\0’
    for( int i = 0; i < buf_size-2; i++ )    buf[i] = 'A' + i % 26;
    buf[buf_size-1] = buf[buf_size-2] = '\0';
    close( fds[0] );		//  只写入，关闭管道读取端
    FILE * stream = fdopen( fds[1], "w" );
    Write( buf, 3, stream );
    close( fds[1] );		//  关闭管道写入端
  }
  return 0;
}
```

##### 管道重定向

等位文件描述符

- 共享相同的文件位置和状态标志设置

`dup()` 函数：将两个文件描述符等位处理

- 原型：
  - `int dup( int oldfd );` 
  - `int dup2( int oldfd, int newfd );`
- 参数：创建 `oldfd` 的一份拷贝，单参数版本选择数值最小的未用文件描述符作为新的文件描述符；双参数版本使用 `newfd` 作为新的文件描述符，拷贝前尝试关闭 `newfd`
- 返回值：成功时返回新文件描述符，失败时返回 -1，并设 `errno` 值
- 示例：`dup2( fd, STDIN_FILENO )` 关闭标准输入流，然后作为 `fd` 的副本重新打开

```c++
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>


const int buf_size = 4096;

int main ()
{
  int fds[2];
  pipe( fds );		//  创建管道
  pid_t pid = fork();
  if( pid == (pid_t)0 )	//  子进程
  {
    close( fds[0] );		//  关闭管道读取端
    dup2( fds[1], STDOUT_FILENO );		//  管道挂接到标准输出流
    char * args[] = { "ls", "-l", "/", NULL };	//  使用“ls”命令替换子进程
    execvp( args[0], args );
  }
  else			//  父进程
  { 
    close( fds[1] );		//  关闭管道写入端
    char buf[buf_size];
    FILE * stream = fdopen( fds[0], "r" );	//  以读模式打开管道读取端，返回文件指针
    fprintf( stdout, "Data received: \n" );
    //  在流未结束，未发生读取错误，且能从流中正常读取字符串时，输出读取到的字符串
    while( !feof(stream) && !ferror(stream) && fgets(buf, sizeof(buf), stream) != NULL )
    {
      fputs( buf, stdout );
    }
    close( fds[0] );		//  关闭管道读取端
    waitpid( pid, NULL, 0 );	//  等待子进程结束
  }
  return 0;
}
```



#### 进程信号量

进程信号量： System V 信号量

- 可以使用同步机制确定进程的先后执行关系
- 头文件：`sys/types.h`、`sys/ipc.h` 和 `sys/sem.h`

信号量的定义

- 信号量是一类特殊的计数器，其值为非负整数，用于进程或线程同步

信号量的操作

- 等待（wait）操作（P）：信号量的值递减 1 后返回；如果值为 0，则阻塞操作，直到信号量值为正（其他进程或线程释放了信号量），然后递减 1 后返回
- 发布（post）操作（V）：信号量的值递增 1 后返回；如果信号量值原为 0，则其中一个等待该信号量的进程或线程取消阻塞

Linux 信号量实现：两个版本

- 进程信号量多用于进程同步
- POSIX标准实现多用于线程同步

使用进程信号量时的注意事项

- 每次创建和管理的进程信号量不是一个，而是一个集合（数组），该集合可能包含多个进程信号量
- 使用键值 `key` 关联进程信号量集，但进程信号量集本身由进程信号量的标识符`semid` 标识，函数调用时几乎总是使用 `semid` ——可以这么理解：`semid` 对内，`key` 对外

##### 获取进程信号量

`semget()` 函数：创建或获取进程信号量集

- 原型：`int semget( key_t key, int nsems, int semflg );`
- 参数：
  - `key` 为键值，多个进程可以通过此键值访问同一进程信号量；
    - 键值 `IPC_PRIVATE` 用于创建当前进程的私有进程信号量集
    - 使用 `IPC_CREAT` 和 `IPC_EXCL` 创建进程信号量集，后者要求创建新的唯一的进程信号量集，若其已存在，则出错
  - `nsems` 为需要创建的进程信号量集的进程信号量元素个数（不是进程信号量的信号数）
    - 要获取已分配的进程信号量集，使用原先键值查询，此时进程信号量集的元素个数可设置为 0
  - `semflg` 为访问标志
- 返回值：成功时返回进程信号量集的标识符 `semid`，失败时返回 -1，并设置 `errno` 值

##### 控制进程信号量

`semctl()` 函数：控制和管理进程信号量集

- 原型：`int semctl( int semid, int semnum, int cmd[, union semun su] );`
- 参数：`semid` 为进程信号量集的标识符，`semnum` 为进程信号量集的元素下标，`cmd` 为指定操作，第四个参数 `arg` 可有可无，与 `cmd` 有关
- 返回值：成功时与 `cmd` 有关，失败时返回 -1，并设置 `errno` 值
- 调用 `semctl()` 函数的进程的有效用户 ID 必须与分配进程信号量集的用户权限匹配

###### 清除进程信号量

释放（删除）进程信号量集：`int semctl( int semid, int semnum=IGNORED, int cmd=IPC_RMID, union semun su=ignored );`

- 最后一个使用进程信号量的进程负责清除进程信号量集

- 进程信号量集释放后，内存自动释放

- 调用说明：使用 `semctl()` 函数进行调用，`cmd` 设置为 `IPC_RMID`，`semnum` 被忽略，`arg` 不需要；如果需要 `arg`，定义 `union semun` 类型的变量并作为参数，部分系统可能未定义 `union semun` 类型，需按如下格式补充定义：

  ```c++
  union semun
  {
    int 			        val;	//  供 SETVAL 使用的值
    struct semid_ds *	    buf;	//  供 IPC_STAT、IPC_SET 使用的缓冲区
    unsigned short int *	array;	//  供 GETALL、SETALL 使用的数组
    struct seminfo *		__buf;	//  供 IPC_INFO 使用的缓冲区
  };
  ```

###### 初始化进程信号量

初始化进程信号量集：`int semctl( int semid, int semnum=0, int cmd=SETALL, union semun su );`

- 第一个参数 `semid` 为进程信号量集的标识符，第二个参数 `semnum` 为 0，第三个参数 `cmd` 为 `SETALL`，第四个参数 `arg` 必须为 `union semun` 类型的数据对象
- `union semun` 的 `array` 字段：指向无符号短整型数组 `unsigned short int values[]` 首元素的指针，该数组保存进程信号量集的所有信号量的信号数

其他常用命令参数

- `IPC_STAT`/`IPC_SET`（获取或设置进程信号量信息）、`GETALL`（获取全部信号量的信号数）、 `GETVAL`/`SETVAL `（获取或设置单个信号量的信号数）等

##### 获取与释放

```c++
//  获取与key关联的二元信号量集，必要时会分配之
int AcquireBinarySemaphore( key_t key, int sem_flags )
{
  return semget( key, 1, sem_flags );
}

//  释放二元信号量集，单一元素
int ReleaseBinarySemaphore( int semid )
{
  union semun  ignored;
  return semctl( semid, 1, IPC_RMID, ignored );
}

//  初始化二元信号量集，单一元素，信号量初始值为1
int InitializeBinarySemaphore( int semid )
{
  unsigned short int values[1] = { 1 };
  union semun needed = { .array = values };
  return semctl( semid, 0, SETALL, needed );
}
```

##### 等待与发布(PV 操作)

等待与发布进程信号量函数 `semop()`

- 原型：`int semop( int semid, struct sembuf * sops, size_t nsops );`
- 参数：`semid` 为待操作的进程信号量集的标识符； `sops` 为操作数组，`nsops` 为操作数组的元素个数
- 返回值：成功时为进程信号量集的标识符，失败时返回 -1，并设置 `errno` 值

`struct sembuf` 类型的成员

- `sem_num`：需要操作的进程信号量集中的信号量元素下标
- `sem_op`：指定信号量操作的整数（递增或递减信号量的信号数）
  - 如果 `sem_op` 为正数，则立即加到信号量上（V 操作）
  - 如果 `sem_op` 为负数，则从信号量上减去（P 操作），如果会使结果为负数，则阻塞进程，直到信号量的信号数不小于 `sem_op` 的绝对值
  - 如果 `sem_op` 为 0，则阻塞进程，直到信号量的信号数为 0

- `sem_flg`：指定 `IPC_NOWAIT` 则不阻塞进程，指定 `SEM_UNDO` 则在进程退出时取消操作

```c++
//  P原语：等待二元信号量，信号数非正时阻塞
int WaitBinarySemaphore( int semid )
{
  struct sembuf ops[1];
  ops[0].sem_num = 0;
  ops[0].sem_op = -1;
  ops[0].sem_flg = SEM_UNDO;
  return semop( semid, ops, 1 );
}

//  V原语：发布二元信号量，增加信号数后立即返回
int PostBinarySemaphore( int semid )
{
  struct sembuf ops[1];
  ops[0].sem_num = 0;
  ops[0].sem_op = 1;
  ops[0].sem_flg = SEM_UNDO;
  return semop( semid, ops, 1 );
}
```

#### 共享内存

共享内存的意义：快捷方便的本地通信机制

- 头文件：`sys/ipc.h` 和 `sys/shm.h`

共享内存编程原则

- 系统没有对共享内存操作提供任何缺省同步行为
- 如果需要，程序员自主设计同步策略：使用进程信号量

共享内存使用过程

- 某个进程分配一个内存段，其他需要访问该内存段的进程连接（attach）该内存段
- 完成访问后，进程拆卸（detach）该内存段
- 某个时刻，一个进程释放该内存段

Linux内存模型

- 每个进程的虚拟地址空间按页（page）编址，页缺省为4096字节（不同硬件架构和操作系统可能不同，使用 `getpagesize()` 函数获取系统值）
- 每个进程维持从内存地址到虚拟页面地址的映射
- 多个进程可能使用同一虚拟页面，同样的数据在不同进程中的地址并不需要相同
- 分配新的共享内存段将创建虚拟内存页面，其他进程连接该共享内存段即可访问
- 共享内存段的分配只有由一个进程负责，释放也同样

##### 获取共享内存

`shmget()` 函数：获取或分配一段共享内存

- 原型：`int shmget( key_t key, size_t size, int shmflg );`
- 参数：`key` 为内存段整数键值，`size` 为内存段分配的字节数（圆整至4096字节整数倍），`shmflg` 为创建共享内存段的位标志

键值参数 `key`

- 其他进程通过键值 `key` 访问该内存段，任意设定的键值可能和其他进程的共享内存段键值冲突，使用 `IPC_PRIVATE` 以确保无冲突

创建标志：`IPC_CREAT`（创建）、`IPC_EXCL`（独占）

- 后者与前者合并使用，如果键值已使用，则创建失败
- 如果未设 `IPC_EXCL`，则在键值已经存在时，返回其代表的共享内存段，而不是创建一个新的共享内存段

位标志参数

- 模式标志：以 9 位数字表示宿主、组用户和其他人的访问控制权
- 常数位于头文件 `sys/stat.h`

返回值：共享内存段的标识符

常用模式常数

- `S_IRUSR` 和 `S_IWUSR` 分别表示共享内存段宿主的读写权限
- `S_IRGRP` 和 `S_IWGRP` 分别表示共享内存段组用户的读写权限
- `S_IROTH` 和 `S_IWOTH` 分别表示共享内存段其他人的读写权限

调用示例

- `int seg_id = shmget( shm_key, getpagesize(), IPC_CREAT | S_IRUSR | S_IWUSER );`

##### 连接与拆卸共享内存

`shmat()` 函数：连接共享内存

- 原型：`void * shmat( int shmid, const void * shmaddr, int shmflg );`
- 参数： `shmid` 为共享内存段标识符（`shmget()` 的返回值）， `shmaddr` 为指针，指向共享内存段的映射地址，如果传递 `NULL`，Linux 自动选择合适地址， `shmflg` 为连接标志
- 返回值：成功时返回所连接的共享内存段的地址

连接标志

- `SHM_RND`：`shmaddr` 指定的映射地址向下圆整到页面尺寸的整数倍；如果未指定，则传递 `shmaddr` 时必须手工对齐页面地址
- `SHM_RDONLY`：共享内存段组只读

`shmdt()` 函数：拆卸共享内存段

- 原型：`int shmdt( const void * shmaddr );`

##### 使用共享内存

```c++
#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
int main ()
{
  struct shmid_ds  shmbuf;
  int seg_size;
  const int shared_size = 0x6400;
  //  分配共享内存段
  int seg_id = shmget( IPC_PRIVATE, shared_size, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR );
  //  连接共享内存段
  char * shared_mem = ( char * )shmat( seg_id, 0, 0 );
  printf( "Shared memory attached at %p\n", shared_mem );
  //  获取段尺寸信息
  shmctl( seg_id, IPC_STAT, &shmbuf );
  seg_size = shmbuf.shm_segsz;
  printf( "Segment size: %d\n", seg_size );
  //  向共享内存区段写入字符串
  sprintf( shared_mem, "Hello, world." );
  //  拆卸共享内存区段
  shmdt( shared_mem );
  //  在不同的地址处重新连接共享内存区段
  shared_mem = ( char * )shmat( seg_id, ( void * )0x5000000, 0 );
  printf( "Shared memory reattached at %p\n", shared_mem );
  //  获取共享内存区段中的信息并打印
  printf( "%s\n", shared_mem );
  //  拆卸共享内存区段
  shmdt( shared_mem );
  //  释放共享内存区段，与semctl类似
  shmctl( seg_id, IPC_RMID, 0 );
  return 0;
}
```



#### 映射内存

`mmap()` 函数：头文件 `sys/mman.h`

- 映射共享文件到内存；文件被分割成页面大小装载；使用内存读写操作访问文件，速度更快；对映射内存的写入自动反映到文件中
- 原型：`void * mmap( void * addr, size_t length, int prot, int flags, int fd, off_t offset );`
- 函数参数
  - `addr`：映射目的地的内存地址，NULL表示由Linux自动选择合适的内存地址
  - `length`：映射内存的大小，以字节为单位
  - `prot`：指定映射内存的保护权限，为 `PROT_READ`（允许读取）、 `PROT_WRITE`（允许写入）、 `PROT_EXEC`（允许执行）或以上三者的组合
  - `flags`：附加选项标志；为 `MAP_FIXED`（如果指定此标志，则Linux使用用户指定的地址映射文件，地址本身必须页对齐）、 `MAP_PRIVATE`（内存写入不回写至外部文件，本进程保留写入的文件副本）、 `MAP_SHARED`（内存写入立即反映到映射文件中）； `MAP_PRIVATE` 与 `MAP_SHARED` 不能混用
  - `fd`：待映射文件的文件描述符
  - `offset`：指定映射数据在文件中的偏移量
- 函数返回值：成功调用时返回映射内存的基地址，失败时返回 `MAP_FAILED`

`munmap()` 函数：释放映射内存

- 原型：`int * munmap( void * addr, size_t length );`

- 参数：`addr` 为映射内存的基地址；`length` 为映射内存的大小

- 返回值：成功时返回 0，失败时返回 -1 并设 `errno` 值

`msync()` 函数：映射内存同步

- 原型：`int msync( void * addr, size_t length, int flags);`

- 参数：`addr` 为映射内存基地址，`length` 为映射内存大小，`flags` 为同步标志，`MS_ASYNC`（数据更新被调度，但函数返回前并不一定会被执行）； `MS_SYNC`（数据更新立即执行，在完成前调用进程被阻塞）； `MS_INVALIDATE`（通知其他进程数据已无效，并自动提供新数据）； `MS_ASYNC` 与 `MS_SYNC` 不能混用

- 返回值：成功时返回 0，失败时返回 -1 并设 `errno` 值

##### 读写映射内存

```c++
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <wait.h>
#include <iostream>
#include <iomanip>
const int mapped_size = 4096;
const int mapped_count = mapped_size / sizeof(int);
int main( int argc, char * const argv[] )
{
  //  打开文件作为内存映射的对象，确保文件尺寸足够存储1024个整数
  int fd = open( argv[1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR );
  lseek( fd, mapped_size - 1, SEEK_SET );
  write( fd, "", 1 );
  lseek( fd, 0, SEEK_SET );
  int * base = ( int * )mmap( 0, mapped_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, fd, 0 );
  close( fd );		//  创建映射内存后，关闭文件的文件描述符
  pid_t pid = fork();
  if( pid == (pid_t)0 )	//  子进程写入数据
  {
    //  写入数据0～1023
    for( int i = 0, * p = base; i < mapped_count; *p++ = i++ )
      ;
    munmap( base, mapped_size );
  }
  else if( pid > (pid_t)0 )	// 父进程读取数据
  {
    sleep( 10 );		//  等待10秒
    for( int i = 0, *p = base; i < mapped_count; i++, p++ )
      std::cout << std::setw(5) << *p << " ";
    std::cout << std::endl;
    munmap( base, mapped_size );
  }
  return 0;
}
```



#### 消息队列

消息队列：在两个进程间传递二进制块数据

- 数据块具有类别信息，接收方可根据消息类别有选择地接收
- 头文件：`sys/type.h`、`sys/ipc.h` 和 `sys/msg.h`

`msgget()` 函数：创建或获取消息队列

- 原型：`int msgget( ket_t key, int msgflg );`
- 参数：`key` 为键值，标识全局唯一的消息队列；`msgflg` 为创建标志，与 `semget()` 的标志相同
- 返回值：成功时返回正整数作为消息队列的标识符，失败时返回 -1，并设 `errno` 值
- 如果用于创建消息队列，相关内核数据结构 `struct msqid_ds` 将被创建并初始化

`msgsnd()` 函数：将消息添加到消息队列中

- 原型：`int msgsnd( int msqid, const void * msg_ptr, size_t msg_sz, int msgflg );`
- 参数： `msqid` 为 `msgget()` 返回的消息队列标识符；`msg_ptr` 指向准备发送的消息； `msg_sz` 为消息数据长度；`msgflg` 控制消息发送行为，一般仅支持`IPC_NOWAIT` 标志，即以非阻塞的方式发送消息

- 返回值：成功时返回 0，失败时返回 -1，并设 `errno` 值

- 消息缓冲区结构
  - `msg_ptr` 指向的数据结构如右
  - `mtype` 为消息类别，必须为正整数
  - `mtext` 为消息数据，`msg_sz` 为其实际长度

`msgrcv()` 函数：从消息队列中获取消息

- 原型：`int msgrcv( int msqid, void * msg_ptr, size_t msg_sz, long int msgtype, int msgflg );`

- 参数：`msqid` 为 `msgget()` 返回的消息队列标识符；`msg_ptr` 用于存储接收的消息；`msg_sz` 为消息数据长度；`msgtype` 为消息类别；`msgflg` 控制消息发送行为，可以为 `IPC_NOWAIT`、`MSG_EXCEPT`（`msgtype` 大于 0 时，读取第一个非 `msgtype` 类别的消息）和 `MSG_NOERROR` 的位或

- 返回值：成功时返回 0，失败时返回 -1，并设 `errno` 值

- 消息类别 `msgtype` 说明：为 0 则读取队列第一条消息，大于 0 则读取队列中第一条类别为 `msgtype` 的消息，小于 0 则读取队列中第一个类别比 `msgtype` 绝对值小的消息

`msgctl()` 函数：控制消息队列的某些属性

- 原型：`int msgctl( int msqid, int cmd, struct msqid_ds * buf );`
- 参数： `msqid` 为 `msgget()` 返回的消息队列标识符；`cmd` 指定要执行的命令，支持的命令有 `IPC_STAT`、`IPC_SET`、`IPC_RMID`、`IPC_INFO`、`MSG_INFO`、`MSG_STAT`；`buf` 的意义与 `cmd` 参数有关
- 返回值：成功时返回值取决于 `cmd` 参数，失败时返回 -1，并设 `errno` 值

#### 进程池

动机：为什么需要引入进程池？

- 进程需要频繁创建子进程，以执行特定任务
- 动态创建子进程的过程效率较低，客户响应速度较慢
- 动态创建的子进程一般只为单一客户提供服务，当客户较多时，系统中会存在大量子进程，进程切换的开销过高
- 动态创建的子进程为当前进程的完整映像，当前进程必须谨慎地管理系统资源，以防止子进程不适当地复制这些资源

什么是进程池？

- 主进程预先创建一组子进程，并统一管理
- 子进程运行同样代码，具有同样属性，个数多与 CPU 数目一致，很少超过 CPU 数目的两倍

进程池的工作原理是什么？

- 主进程充当服务器，子进程充当服务员，按照服务器的需要提供服务
- 在任务到达时，主进程选择一个子进程进行服务
- 相对于动态创建子进程，选择的代价显然更小——这些子进程未来还可以被复用
- 存在多种选择子进程的策略，如随机选择或轮值制度，如共享单一任务队列，还可以使用更加智能化的负载平衡技术
- 父子进程之间应该具有传递信息的通道，如管道、共享内存、消息队列等，也可能需要同步机制

### 进程编程实践

13.1、编写程序，调用 `fork()` 创建子进程，使用二元进程信号量进行同步。提示：在创建子进程前，使用 `IPC_PRIVATE` 创建新的二元进程信号量，其创建的进程信号量并不是该进程私有的，子进程可以通过复制的 `semid` 访问该二元进程信号量。父进程在等待子进程结束后释放该二元进程信号量。

13.2、编程实现进程池类 `ProcessPool`。提示：（1）进程池应实现为类模板，从而可以针对不同的任务类别构造不同的进程池；（2）每个任务类别的进程池应实现为单子类；（3）可以统一事件源，即统一管理同类的任务序列，典型的策略是实现父子进程通信的信号管道；（4）调度算法随意。



## 线程编程



### 线程基本概念

线程的定义

- 线程是比进程更小的程序执行单位
- 多个线程可共享全局数据，也可使用专有数据

Linux线程支持史

- 1996年，LinuxThreads：基本符合 POSIX 标准，但效率低下，问题多多
- 2003年，内核 2.6：提供线程支持库 NPTL（Native POSIX Thread Library for Linux）

内核线程

- 操作系统内核支持多线程调度与执行
- 内核线程使用资源较少，仅包括内核栈和上下文切换时需要的保存寄存器内容的空间

轻量级进程（lightweight process，LWP）

- 由内核支持的独立调度单元，调度开销小于普通的进程
- 系统支持多个轻量级进程同时运行，每个都与特定的内核线程相关联

用户线程

- 建立在用户空间的多个用户级线程，映射到轻量级进程后调度执行
- 用户线程在用户空间创建、同步和销毁，开销较低
- 每个线程具有独特的 ID

使用说明

- 线程功能不属于 C/C++ 标准库，链接时需用 `-pthread` 选项
- 线程功能属于 C++11 标准库，可用 C++11 提供的 thread 类定义线程对象，C++11 标准库同时提供基本的线程同步机制

进程与线程的比较

- 线程空间不独立，有问题的线程会影响其他线程；进程空间独立，有问题的进程一般不会影响其他进程
- 创建进程需要额外的性能开销
- 线程用于开发细颗粒度并行性，进程用于开发粗颗粒度并行性
- 线程容易共享数据，进程共享数据必须使用进程间通讯机制

### 线程管理

#### 线程创建

线程创建函数

- 头文件：`pthread.h`
- 原型：`int pthread_create( pthread_t * thread, const pthread_attr_t * attr, void * ( *start_routine )( void * ), void * arg );`

线程创建流程

- 定义指向 `pthread_t` 对象的指针对象，`pthread_t` 对象用于存储新线程的 ID
- 定义指向线程属性 `pthread_attr_t` 对象的指针对象；线程属性对象控制线程与程序其他部分（可能是其他线程）的交互；如果传递 `NULL`，则使用缺省属性构造新线程
- 定义指向线程函数的指针对象，使其指向固定格式的线程函数
- 实现线程函数；线程函数的参数和返回值均为哑型指针；需要传递多个参数时，打包成单个 `void*` 型的指针对象
- 线程退出时使用返回值将数据传递给主调线程；多个结果同样可以打包传递

线程创建说明

- `pthread_create()` 函数在线程创建完毕后立即返回，它并不等待线程结束
- 原线程与新线程如何执行与调度有关，程序不得依赖线程先后执行的关系
- 可以使用同步机制确定线程的先后执行关系

线程退出方式

- 线程函数结束执行
- 调用 `pthread_exit()` 函数显式结束
- 被其他线程撤销

```c++
#include <pthread.h>
#include <iostream>
void *  PrintAs( void * unused )
{
  while( true )    std::cerr << 'a';
  return NULL;
}
void *  PrintZs( void * unused )
{
  while( true )    std::cerr << 'z';
  return NULL;
}
int  main()
{
  pthread_t  thread_id;
  pthread_create( &thread_id, NULL, &PrintAs, NULL );
  PrintZs( NULL );
  return 0;
}
```

#### 线程函数参数

```c++
#include <pthread.h>
#include <iostream>

class InfoPrinted
{
public:
  InfoPrinted( char c, int n ) : _c(c), _n(n)  {  }
  void  Show() const  {  for( int i = 0; i < _n; i++ )  std::cerr << _c;  }
private:
  char _c;
  int _n;
};

void *  PrintInfo( void * info )
{
  InfoPrinted *  p = reinterpret_cast<InfoPrinted *>( info );
  if( p )    p->Show();
  return NULL;
}

//  注意：本程序大部分情况下不会输出任何结果
int  main()
{
  pthread_t  tid1, tid2;
  //  构造InfoPrinted类的动态对象，作为线程函数参数传递给线程tid1
  //  输出100个‘a’
  InfoPrinted *  p = new InfoPrinted( 'a', 100 );
  pthread_create( &tid1, NULL, &PrintInfo, reinterpret_cast<void *>( p ) );
  //  构造InfoPrinted类的动态对象，作为线程函数参数传递给线程tid2
  //  输出100个‘z’
  InfoPrinted *  q = new InfoPrinted( 'z', 100 );
  pthread_create( &tid2, NULL, &PrintInfo, reinterpret_cast<void *>( q ) );
  //  使用本注释行替换上述线程，可以看到输出结果，可能仅有部分输出
  //  PrintInfo( reinterpret_cast<void *>( q ) );
  return 0;
}
```

存在的问题：一般不会产生任何输出

- 子线程需要使用主线程的数据，如果主线程结束，子线程如何访问这些数据？

解决方案：使用 `pthread_join()` 函数，等待子线程结束

- 原型：`int pthread_join( pthread_t thread, void ** retval );`
- 参数：`thread` 为 `pthread_t` 类型的线程 ID；`retval` 接收线程返回值，不需要接收返回值时传递 `NULL`

```c++
//  注意：无法确定两个线程的执行顺序，多次输出结果可能不同
int  main()
{
  pthread_t  tid1, tid2;

  InfoPrinted *  p = new InfoPrinted( 'a', 100 );
  pthread_create( &tid1, NULL, &PrintInfo, reinterpret_cast<void *>( p ) );

  InfoPrinted *  q = new InfoPrinted( 'z', 100 );
  pthread_create( &tid2, NULL, &PrintInfo, reinterpret_cast<void *>( q ) );

  //  等待子线程结束
  pthread_join( tid1, NULL );
  pthread_join( tid2, NULL );

  return 0;
}
```

#### 线程函数返回值

```c++
#include <pthread.h>
#include <cmath>
#include <iostream>

void *  IsPrime( void * n )
{
  unsigned int  p = reinterpret_cast<unsigned int>( n );
  unsigned int  i = 3u, t = (unsigned int)sqrt( p ) + 1u;
  if( p == 2u )
    return reinterpret_cast<void *>( true );
  if( p % 2u == 0u )
    return reinterpret_cast<void *>( false );
  while( i <= t )
  {
    if( p % i == 0u )
      return reinterpret_cast<void *>( false );
    i += 2u;
  }
  return reinterpret_cast<void *>( true );
}

//  使用g++ main.cpp –pthread –lm –fpermissive编译
//  以防止编译器将void*到int的转型当作错误
int  main()
{
  pthread_t  tids[8];
  bool  primalities[8];
  int i;
  for( i = 0; i < 8; i++ )
    pthread_create( &tids[i], NULL, &IsPrime, reinterpret_cast<void *>( i+2 ) );
  for( i = 0; i < 8; i++ )
    pthread_join( tids[i], reinterpret_cast<void **>( &primalities[i] ) );
  for( i = 0; i < 8; i++ )
    std::cout << primalities[i] << " ";
  std::cout << std::endl;
  return 0;
}
```

#### 线程 ID

`pthread_equal()` 函数：确认两个线程是否相同

- 原型：`int pthread_equal( pthread_t t1, pthread_t t2 );`

`pthread_self()` 函数：返回当前线程的 ID

- 原型：`pthread_t pthread_self();`
- 示例：`if( !pthread_equal( pthread_self(), other_tid ) )  pthread_join( other_tid, NULL );`

#### 线程属性

线程属性：精细调整线程的行为

设置线程属性的流程

- 创建 `pthread_attr_t` 类型的对象
- 调用 `pthread_attr_init()` 函数初始化线程的缺省属性，传递指向该线程属性对象的指针
  - 原型：`int pthread_attr_init( pthread_attr_t * attr );`
- 对线程属性进行必要修改
- 调用 `pthread_create()` 函数时传递指向线程属性对象的指针
- 调用 `pthread_attr_destroy()` 函数清除线程属性对象， `pthread_attr_t`对象本身没有被销毁，因而可以调用 `pthread_attr_init()` 函数再次初始化
  - 原型：`int pthread_attr_destroy( pthread_attr_t * attr );`

线程属性说明

- 单一线程属性对象可以用于创建多个线程
- 线程创建后，继续保留线程属性对象本身并没有意义
- 对大多数 Linux 程序，线程最重要的属性为分离状态（detach state）

线程分类

- 可联线程（joinable thread）：缺省设置，终止时并不自动清除（类似僵尸进程），主线程必须调用 `pthread_join()` 获取其返回值，此后才能清除
- 分离线程（detached thread）：结束时自动清除，不能调用 `pthread_join()` 进行线程同步
- 可联线程可通过 `pthread_detach()` 函数分离，分离线程不能再次联结
  - 原型：`int pthread_detach( pthread_t thread );`

`pthread_attr_setdetachstate()` 函数：设置线程分离属性

- 原型：`int pthread_attr_setdetachstate ( pthread_attr_t * attr, int detachstate );`
- 传递线程属性对象指针和分离线程设置参数`PTHREAD_CREATE_DETACHED`

`pthread_attr_getdetachstate()` 函数：获取线程分离属性

- 原型：`int pthread_attr_getdetachstate ( pthread_attr_t * attr, int * detachstate );`

```c++
#include <pthread.h>
//  线程函数
void *  ThreadFunc( void * arg )  {  ...  }
int  main()
{
  pthread_attr_t  attr;
  pthread_t  thread;
  //  初始化线程属性
  pthread_attr_init( &attr );
  //  设置线程属性的分离状态
  pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );
  //  创建线程
  pthread_create( &thread, &attr, &ThreadFunc, NULL );
  //  清除线程属性对象
  pthread_attr_destroy( &attr );
  //  无需联结该线程
  return 0;
}
```

#### 线程撤销

`pthread_cancel()` 函数：撤销线程

- 原型：`int pthread_cancel( pthread_t thread );`
- 已撤销的线程可以联结，且必须联结，以释放其资源，除非其为分离线程

线程撤销类型与状态

- 异步可撤销：在其执行的任何时刻都可撤销
- 同步可撤销：线程可撤销，但撤销操作首先进入队列排队，在线程执行到特定撤销点时才可撤销
- 不可撤消：撤销不可撤消线程的企图被系统忽略，且没有任何消息反馈

`pthread_setcanceltype()` 函数：设置线程的撤销类型

- 原型：`int pthread_setcanceltype( int type, int * oldtype );`
- 参数：`type` 为撤销类型，`oldtype` 用于保存原始线程撤销类型，`NULL` 表示不保存
- `PTHREAD_CANCEL_ASYNCHRONOUS` ：线程异步可撤销
- `PTHREAD_CANCEL_DEFERRED`：线程同步可撤销，即延迟到下一撤销点时撤销

`pthread_setcancelstate()` 函数：设置线程的撤销状态

- 原型： `int pthread_setcancelstate( int state, int * oldstate );`
- 第一个参数 `state` 为可撤销状态，第二个参数 `oldstate` 用于保存原始线程可撤销状态，`NULL` 表示不保存
- `PTHREAD_CANCEL_ENABLE`：线程可撤销
- `PTHREAD_CANCEL_DISABLE`：线程不可撤销
- 线程的撤销状态可多次设置

`pthread_testcancel()` 函数：设置撤销点

- 原型：`void pthread_testcancel();`
- 在线程函数中调用 `pthread_testcancel()` 函数设置撤销点
- 建议：周期性地设置撤销点，保证线程函数内部每隔一些代码就有一个撤销点，以保证资源能够正确释放

使用撤销状态构造临界区（critical section）

- 临界区：要么全部执行，要么一条都不执行的代码段
- 设置线程的撤销状态，线程一旦进入临界区，就必须等到离开临界区，才可以被撤销

```c++
//  账户转账
void Transfer( double * accounts, int from, int to, double amount )
{
  int ocs;
  //  数据有效性检查代码在此，确保转账操作合法有效

  //  将线程设置为不可撤销的，进入临界区
  pthread_setcancelstate( PTHREAD_CANCEL_DISABLE, &ocs );

  accounts[to] += amount;
  accounts[from] -= amount;

  //  恢复线程的撤销状态，离开临界区
  pthread_setcancelstate( ocs, NULL );
}
```

#### 线程局部存储

线程局部存储（thread local storage，TLS）：每个线程的独有数据

- 线程特定数据（thread-specific data）
- 进程的多个线程通过全局堆共享全局数据对象
- 每个线程拥有独立的栈

让线程拥有数据的独立副本：不能简单赋值或读取

- `pthread_key_create()` 函数：为线程特定数据创建一个键
- 参数：第一个为指向 `pthread_key_t` 类型变量的指针（每个线程都可以使用它访问自己的独立数据副本）；第二个参数为指向线程清除函数的指针，如果不存在，传递 `NULL`
- `pthread_setspecific()` 函数：设置对应键的值
- `pthread_getspecific()` 函数：读取对应键的值

```c++
#include <pthread.h>
#include <stdio.h>
static pthread_key_t  tlk;    //  关联线程日志文件指针的键
void  WriteToThreadLog( const char * msg )
{
  FILE *  fp = ( FILE * )pthread_getspecific( tlk );
  fprintf( fp, "%d: %s\n", (int)pthread_self(), msg );
}
void  CloseThreadLog( void * fp )
{
  fclose( ( FILE * )fp );
}
void *  ThreadFunc( void * args )
{
  char  filename[255];
  FILE *  fp;
  //  生成与线程ID配套的日志文件名
  sprintf( filename, "thread%d.log", (int)pthread_self() );
  fp = fopen( filename, "w" );
  //  设置线程日志文件指针与键的局部存储关联
  pthread_setspecific( tlk, fp );
  //  向日志中写入数据，不同的线程会写入不同的文件
  WriteToThreadLog( "Thread starting..." );
  return NULL;
}
int  main()
{
  int  i;
  pthread_t  threads[8];
  //  创建键，使用CloseThreadLog()函数作为其清除程序
  pthread_key_create( &tlk, CloseThreadLog );
  for( i = 0; i < 8; ++i )
    pthread_create( &threads[i], NULL, ThreadFunc, NULL );
  for( i = 0; i < 8; ++i )
    pthread_join( threads[i], NULL );
  pthread_key_delete( tlk );
  return 0;
}
```

#### 线程清除

线程清除函数：回调函数，单 `void*` 参数，无返回值

- 目的：销毁线程退出或被撤销时未释放的资源

`pthread_cleanup_push()` 函数：注册线程清除函数

- 原型： `void pthread_cleanup_push( void (*routine)(void*), void * arg );`
- 参数：`routine` 为指向线程清除函数的函数指针，`arg` 为传递给回调函数的附加数据对象

`pthread_cleanup_pop()` 函数：取消线程清除函数注册

- 原型： `void pthread_cleanup_pop( int execute );`
- 参数：整型值，非 0 调用回调函数释放资源，0 不释放

```c++
#include <malloc.h>
#include <pthread.h>
void *  AllocateBuffer( size_t size )
{
  return malloc( size );
}
void  DeallocateBuffer( void * buffer )
{
  free( buffer );
}
void  DoSomeWork()
{
  void *  temp_buffer = AllocateBuffer( 1024 );
  //  注册清除处理函数
  pthread_cleanup_push( DeallocateBuffer, temp_buffer );
  //  此处可以调用pthread_exit()退出线程或者撤销线程
  //  取消注册，传递非0值，实施清除任务
  pthread_cleanup_pop( 1 );
}
```

C++的问题

- 对象的析构函数在线程退出时可能没有机会被调用，因而线程栈上的数据未清除
- 如何保证线程资源被正确释放？

解决方法

- 定义异常类，线程在准备退出时引发异常，然后在异常处理中退出线程执行
- 引发异常时，C++确保析构函数被调用

```c++
#include <pthread.h>
class EThreadExit  {
public:
  EThreadExit( void * ret_val ) : _thread_ret_val(ret_val)  {  }
  //  实际退出线程，使用对象构造时的返回值
  void* DoThreadExit ()  {  pthread_exit( _thread_ret_val );  }
private:
  void *  _thread_ret_val;
};
void *  ThreadFunc( void * arg )
{
  try  {
    if( 线程需要立即退出 )
      throw EThreadExit(  线程返回值 );
  }
  catch( const EThreadExit & e )  {
    e.DoThreadExit();    //  执行线程实际退出动作
  }
  return NULL;
}
```



### 线程同步机制

#### 资源竞争

编程任务

- 存在一个任务队列，多个并发线程同时处理这些任务。每个线程在完成某项任务后，检查任务队列中是否有新任务。如果有，就处理该任务，并将该任务从任务队列中删除。
- 假设：两个线程碰巧完成各自任务，但队列中只有一个任务。
- 可能发生的情况：第一个线程发现任务队列非空，准备接收该任务，但没有完成全部设置。此时，操作系统碰巧中断该线程。第二个线程获得了执行，也发现任务队列非空，同样准备接收该任务，但发现已无法正确设置任务队列。
- 最坏情况：第一个线程已经从队列中摘取了任务，但是还没有将任务队列设置为空，第二个线程对任务队列的访问导致段错误，系统崩溃。

```c++
//  有问题的程序代码
#include <list>
struct Job;
std::list<Job *>  job_queue;
//  线程函数
void *  DequeueJob( void * arg )
{
  if( !job_queue.empty() )
  {
    Job *  job = job_queue.front();
    job_queue.pop_front();
    ProcessJob( job );
    delete job,  job = NULL;
  }
  return NULL;
}
```

#### 互斥

互斥（mutex）定义与性质：MUTial EXclusion

- 相互独占锁，与二元信号量类似
- 一次只有一个线程可以锁定一个数据对象，并访问
- 只有该线程释放锁定，其他线程才能访问该数据对象

`pthread_mutex_init()` 函数：初始化互斥

- 原型：`int pthread_mutex_init( pthread_mutex_t * mutex, const pthread_mutexattr_t * mutexattr );`
- 参数： `mutex` 为互斥对象，`mutexattr` 为互斥属性对象，`NULL` 表示使用缺省属性
- 可使用预定义宏 `PTHREAD_MUTEX_INITIALIZER` 初始化互斥

`pthread_mutex_destroy()` 函数：销毁互斥

- 原型：`int pthread_mutex_destroy( pthread_mutex_t * mutex );`

`pthread_mutex_lock()` 函数：互斥加锁

- 原型：`int pthread_mutex_lock( pthread_mutex_t * mutex );` 
- 如果无法锁定，则调用将阻塞，至该互斥被解除锁定状态

`pthread_mutex_trylock()` 函数：互斥加锁

- 原型：`int pthread_mutex_trylock( pthread_mutex_t * mutex );` 
- 如果无法锁定，则立即返回，不阻塞

`pthread_mutex_unlock()` 函数：互斥解锁

- 原型：`int pthread_mutex_unlock( pthread_mutex_t * mutex);`

使用互斥的流程

- 定义 `pthread_mutex_t` 类型的变量，将其地址作为第一个参数传给 `pthread_mutex_init()` 函数；初始化函数只需调用一次

- 锁定或尝试锁定该互斥；获得访问权后，执行正常程序代码；并在执行完毕后解锁

互斥属性

- `pshared` 属性：进程共享属性
  - 取值：`PTHREAD_PROCESS_SHARED`（跨进程共享），`PTHREAD_PROCESS_PRIVATE`（本进程内部共享）
- `type` 属性：互斥类型

互斥type属性

- PTHREAD_MUTEX_NORMAL：普通锁
  - 被某个线程锁定后，其他请求加锁的线程将等待
  - 容易导致死锁
  - 解锁被其他线程锁定或已解锁的互斥，将导致不可预期的后果
- PTHREAD_MUTEX_ERRORCHECK：检错锁
  - 线程对已被其他线程锁定的互斥加锁，将返回EDEADLK
- PTHREAD_MUTEX_RECURSIVE：递归锁
  - 允许线程对互斥多次加锁；解锁次数必须与加锁次数匹配
- PTHREAD_MUTEX_DEFAULT：默认锁
  - 实现上可能为上述三种之一

互斥属性函数

- 初始化互斥属性对象：`int pthread_mutexattr_init( pthread_mutexatt_t * attr );`

- 销毁互斥属性对象：`int pthread_mutexattr_destroy( pthread_mutexatt_t * attr );`

- 获取 `pshared` 属性：`int pthread_mutexattr_getpshared( const pthread_mutex_t * mutex, int * pshared );`

- 设置 `pshared` 属性：`int pthread_mutexattr_setpshared( pthread_mutex_t * mutex, int pshared );`

- 获取 `type` 属性：`int pthread_mutexattr_gettype( const pthread_mutex_t * mutex, int * type );`

- 设置 `type` 属性：`int pthread_mutexattr_settype( pthread_mutex_t * mutex, int type );`

```c++
//  完整程序代码
#include <pthread.h>
#include <iostream>
#include <list>
struct Job  {
  Job( int x = 0, int y = 0) : x(x), y(y)  {  }
  int x, y;
};
//  一般要求临界区代码越短越好，执行时间越短越好，使用C++ STL可能并不是好选择
std::list<Job *>   job_queue;
pthread_mutex_t   job_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
//  此处作业处理工作仅为示例，简单输出线程ID和作业内容信息
void  ProcessJob( Job * job )
{
  std::cout << "Thread " << (int)pthread_self();
  std::cout << " processing (" << job->x << ", " << job->y << ")\n";
}
//  处理作业时需要加锁
void *  DequeueJob( void * arg )
{
  while( true )  {
    Job *  job = NULL;
    pthread_mutex_lock( &job_queue_mutex );
    if( !job_queue.empty() )    {
      job = job_queue.front();	//  获取表头元素
      job_queue.pop_front();	//  删除表头元素
    }
    pthread_mutex_unlock( &job_queue_mutex );
    if( !job )    break;
    ProcessJob( job );
    delete job,  job = NULL;
  }
  return NULL;
}

//  作业入队时需要加锁
void *  EnqueueJob( void * arg )
{
  Job * job = reinterpret_cast< Job * >( arg );
  pthread_mutex_lock( &job_queue_mutex );    //  锁定互斥
  job_queue.push_back( job );

  //  入队时也输出线程ID和作业内容信息
  std::cout << "Thread " << (int)pthread_self();
  std::cout << " enqueueing (" << job->x << ", " << job->y << ")\n";

  pthread_mutex_unlock( &job_queue_mutex );    //  解锁
  return NULL;
}

int  main()
{
  int  i;
  pthread_t  threads[8];
  for( i = 0; i < 5; ++i )
  {
    Job *  job = new Job( i+1, (i+1)*2 );
    pthread_create( &threads[i], NULL, EnqueueJob, job );
  }
  for( i = 5; i < 8; ++i )
    pthread_create( &threads[i], NULL, DequeueJob, NULL );
  for( i = 0; i < 8; ++i )
    pthread_join( threads[i], NULL );
  return 0;
}
```

#### 死锁

死锁：资源被竞争占用，且无法释放

处理策略：更改互斥类型

- 创建互斥属性 `pthread_mutexattr_t` 型的对象
- 调用 `pthread_mutexattr_init()` 函数初始化互斥属性对象，传递其地址
- 调用 `pthread_mutexattr_setkind_np()` 函数设置互斥类型，函数第一个参数为指向互斥属性对象的指针，第二个参数为 `PTHREAD_MUTEX_RECURSIVE_NP`（递归互斥）或 `PTHREAD_MUTEX_ERRORCHECK_NP`（检错互斥）
- 调用 `pthread_mutexattr_destroy()` 函数销毁互斥属性对象

#### 信号量

问题：如何确保任务队列中有任务可以做？

- 如果队列中没有任务，线程可能退出，后续任务出现时，没有线程可以执行它

POSIX 标准信号量：头文件 `semaphore.h`

- 用于多个线程的同步操作
- 操作方法比进程信号量简单

初始化信号量

- 原型：`int sem_init( sem_t * sem, int pshared, unsigned int value );`
- 参数：`sem` 为信号量对象，`pshared` 为共享属性，`value` 为信号量初始值

等待信号量：P 操作

- 原型：`int sem_wait( sem_t * sem );`
- 原型：`int sem_trywait( sem_t * sem );`
- 原型：`int sem_timewait( sem_t * sem, const struct timespec * abs_timeout );`
- 说明：`sem_wait()` 在无法操作时阻塞， `sem_trywait()` 则立即返回，`sem_timewait()` 与 `sem_wait()` 类似，但有时间限制

发布信号量：V 操作

- 原型：`int sem_post( sem_t * sem );`

销毁信号量

- 原型：`int sem_destroy( sem_t * sem );`

#### 作业队列

```c++
//  完整程序代码
#include <pthread.h>
#include <iostream>
#include <list>
struct Job  {
  Job( int x = 0, int y = 0) : x(x), y(y)  {  }
  int x, y;
};
std::list<Job *>   job_queue;
pthread_mutex_t   job_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

//  控制作业数目的信号量
sem_t   job_queue_count;

void  ProcessJob( Job * job )
{
  std::cout << "Thread " << (int)pthread_self();
  std::cout << " processing (" << job->x << ", " << job->y << ")\n";
}

//  处理作业时需要加锁
void *  DequeueJob( void * arg )
{
  while( true )  {
    Job *  job = NULL;
    sem_wait( &job_queue_count );	//  等待作业队列中有新作业
    pthread_mutex_lock( &job_queue_mutex );
    if( !job_queue.empty() )  {
      job = job_queue.front();	//  获取表头元素
      job_queue.pop_front();		//  删除表头元素
    }
    pthread_mutex_unlock( &job_queue_mutex );
    if( !job )    break;
    ProcessJob( job );
    delete job,  job = NULL;
  }
  return NULL;
}

//  作业入队时需要加锁
void *  EnqueueJob( void * arg )
{
  Job *  job = reinterpret_cast< Job * >( arg );
  pthread_mutex_lock( &job_queue_mutex );    //  锁定互斥
  job_queue.push_back( job );

  //  新作业入队，递增信号量
  sem_post( &job_queue_count );

  //  入队时也输出线程ID和作业内容信息
  std::cout << "Thread " << (int)pthread_self();
  std::cout << " enqueueing (" << job->x << ", " << job->y << ")\n";

  pthread_mutex_unlock( &job_queue_mutex );    //  解锁
  return NULL;
}

int  main()
{
  int  i;
  pthread_t  threads[8];
  if( !job_queue.empty() )    job_queue.clear();
  sem_init( &job_queue_count, 0, 0 );	//  初始化，非进程共享，初始值0
  for( i = 0; i < 5; ++i )
  {
    Job *  p = new Job( i+1, (i+1)*2 );
    pthread_create( &threads[i], NULL, EnqueueJob, p );
  }
  for( i = 5; i < 8; ++i )
    pthread_create( &threads[i], NULL, DequeueJob, NULL );
  for( i = 0; i < 8; ++i )
    pthread_join( threads[i], NULL );		//  等待线程终止，无作业时线程被阻塞
  sem_destroy( &job_queue_count );	//  销毁作业信号量
  return 0;
}
```



#### 条件变量

条件变量的功能与目的

- 互斥用于同步线程对共享数据对象的访问
- 条件变量用于在线程间同步共享数据对象的值

初始化条件变量

- 原型：`int pthread_cond_init( pthread_cond_t * cond, const pthread_condattr_t * cond_attr );`
- 可使用宏 `PTHREAD_COND_INITIALIZER` 代替

销毁条件变量

- 原型：`int pthread_cond_destroy( pthread_cond_t * cond );`

广播条件变量

- 原型：`int pthread_cond_broadcast( pthread_cond_t * cond );`
- 以广播方式唤醒所有等待目标条件变量的线程

唤醒条件变量

- 原型：`int pthread_cond_signal( pthread_cond_t * cond );`

等待条件变量

- 原型：`int pthread_cond_wait( pthread_cond_t * cond, pthread_mutex_t * mutex );`
- 参数：`mutex` 为互斥，以确保函数操作的原子性

### C++11线程库

支持平台无关的并行程序开发

库：`atomic`、`thread`、`mutex`、`condition_variable`、`future`

- `thread`：`std::thread` 类与 `std::this_thread` 名空间
- `mutex`：互斥相关类，包括 `std::mutex` 系列类，`std::lock_guard` 类、`std::unique_lock` 类及其他型式和函数
- `condition_variable`：条件变量类，包括  `std::condition_variable` 类与 `std::condition_variable_any` 类
- `atomic：std::atomic` 类与 `std::atomic_flag` 类，另外还有一套 C 风格的原子型式和原子操作函数
- `future`：包含两个承诺类（`std::promise` 类、`std::packaged_task` 类）、两个期许类（`std::future` 类、`std::shared_future` 类）及相关型式和函数

参考文献

[Anthony Williams. C++ Concurrency in Action, Practical Multithreading. Manning Publications, 2012.](https://www.gitbook.com/book/chenxiaowei/cpp_concurrency_in_action/details)

#### 线程类

线程类：`thread`

- 支持的线程函数无参数和返回值型式的特别要求，有无参数均可，返回值有无亦可

与Linux线程机制相比，C++11 线程类更易用

线程局部存储使用 `thread_local` 关键字

可派生自己的 `thread` 类，但实现上需特别注意

- 线程类应支持移动语义，但不应支持拷贝语义

常用线程类成员函数

- 判断线程是否可联：`bool thread::joinable();`
- 等待线程结束：`void thread::join();`
- 分离线程：`void thread::detach();`

定义于名空间 `this_thread` 的线程管理函数

- 获取线程 ID：`thread::id get_id();`

- 在处于等待状态时，让调度器选择其他线程执行：`void yield();`

- 阻塞当前线程指定时长：

  ````c++
  template<typename _Rep, typename _Period> 
  void sleep_for( const chrono::duration<_Rep, _Period>& __rtime );
  ````

- 阻塞当前线程至指定时点：

  ```c++
  template<typename _Clock, typename _Duration> 
  void sleep_until( const chrono::time_point<_Clock, _Duration>& __atime );
  ```


无参数线程函数
```c++
#include <iostream>
#include <thread>

void  ThreadFunc()
{
  std::cout << "Thread ID: " << std::this_thread::get_id() << std::endl;
}

int  main()
{
  std::thread  t( &ThreadFunc );		//  创建线程对象并运行
  t.join();				//  等待线程结束
  return 0;
}
```

带双参数的线程函数

```c++
#include <iostream>
#include <thread>
void  ThreadFunc( int a, int b )
{
  std::cout << "Thread ID: " << std::this_thread::get_id() << std::endl;
  std::cout << a << " + " << b << " = " << a + b << std::endl;
}
int main()
{
  int  m = 10, n = 20;
  //  C++11标准库使用可变参数的模板形式参数列表，线程函数参数个数任意
  std::thread  t( &ThreadFunc, m, n );
  t.join();
  return 0;
}
```

带双参数的函子对象

```c++
#include <iostream>
#include <thread>

class Functor  {
public:
  void  operator()( int a, int b )  {
    std::cout << "Thread ID: " << std::this_thread::get_id() << std::endl;
    std::cout << a << " + " << b << " = " << a + b << std::endl;
  }
};

int main()
{
  int  m = 10, n = 20;
  std::thread  t( Functor(), m, n );
  t.join();
  return 0;
}
```

使用 `std::bind()` 函数绑定对象及其普通成员函数

```c++
#include <iostream>
#include <thread>
#include <functional>

class Worker  {
public:
  Worker( int a = 0, int b = 0 ) : _a(a), _b(b)  {  }
  void  ThreadFunc()  {  ……  }
private:  int  _a, _b;
};

int  main()
{
  Worker  worker( 10, 20 );
  std::thread  t( std::bind( &Worker::ThreadFunc, &worker ) );
  t.join();
  return 0;
}
```



#### 互斥类

基本互斥：`mutex` 类

- 核心成员函数 `lock()`、`try_lock()` 和 `unlock()`
- 上述成员函数无参数，无返回值

递归互斥：`recursive_mutex` 类

- 允许单个线程对互斥进行多次加锁与解锁处理

定时互斥：`timed_mutex` 类

- 在某个时段里或者某个时刻前获取互斥
- 当线程在临界区操作的时间非常长，可以用定时锁指定时间

定时递归互斥：`recursive_timed_mutex` 类

- 综合 `timed_mutex` 和 `recursive_mutex`

共享定时互斥：`shared_timed_mutex` 类（C++14）

```c++
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
std::mutex  x;
void  ThreadFunc()
{
  x.lock();
  std::cout << std::this_thread::get_id() << " is entering..." << std::endl;
  std::this_thread::sleep_for( std::chrono::seconds( 3 ) );
  std::cout << std::this_thread::get_id() << " is leaving..." << std::endl;
  x.unlock();
}
int  main()
{
  std::vector<std::thread *>  v( 8 );
  for( int i = 0; i < 8; i++ )    v[i] = new std::thread( ThreadFunc );
  for( int i = 0; i < 8; i++ )    v[i]->join();
}
```

互斥的问题：容易导致死锁

- 若某个线程在临界区内的操作导致异常，有可能无法解锁，从而导致其他线程被永久阻塞
- 若临界区代码有多路分支，其中部分分支提前结束，但没有执行解锁操作，其他线程依然被永久阻塞
- 当多个线程同时申请多个资源时，加锁次序不同也可能导致死锁

资源获取即初始化（resource acquisition is initialization, RAII）

- 使用互斥对象管理类模板自动管理资源

基于作用域的锁管理类模板：`std::lock_guard`

- 构造时是否加锁可选，不加锁时假定当前线程已获得锁的所有权，析构时自动解锁，所有权不可转移，对象生存期内不允许手动加锁和解锁

独一锁管理类模板：`std::unique_lock`

- 构造时是否加锁可选，对象析构时如果持有锁会自动解锁，所有权可转移，对象生存期内允许手动加锁和解锁

共享锁管理类模板：`std::shared_lock`（C++14）

- 用于管理可转移和共享所有权的互斥对象

互斥管理策略

- 延迟：`std::defer_lock`，构造互斥管理对象时延迟加锁操作
- 尝试：`std::try_to_lock`，构造互斥管理对象时尝试加锁操作，但不阻塞线程，互斥不可用时立即返回
- 接收：`std::adopt_lock`，假定当前线程已获得互斥所有权，不再加锁
- 缺省行为：构造互斥管理对象时没有传递管理策略标签参数，阻塞当前线程至成功获得互斥

互斥的解锁时机

- 当使用 C++11 的互斥自动管理策略时，只有析构互斥管理对象时才自动释放互斥，因此要特别注意互斥的持有时间；若线程持有互斥的时间过长，有可能极大降低程序效率
- 解决方案：使用复合语句块或专用辅助函数封装临界区操作；动态创建互斥管理对象，并尽早动态释放

多个互斥的竞争访问

- 多个线程对多个互斥加锁时保持顺序一致性，以避免可能的死锁
- 使用 `std::lock()` 或 `std::try_lock()`


使用互斥管理策略类重新实现线程函数

```c++
template< typename T > 
class Worker
{
public:
  explicit Worker( int no, T a = 0, T b = 0 ) : _no(no), _a(a), _b(b)  {  }
  void  ThreadFunc( T * r )
  {
    {    // 使用复合语句块封装临界区操作，块结束时即释放局部对象
      std::lock_guard<std::mutex>  locker( x );    //  构造对象的同时加锁
      *r = _a + _b;
    }    //  无需手工解锁，locker对象在析构时自动解锁
    std::cout << "Thread No: " << _no << std::endl;
    std::cout << _a << " + " << _b << " = " << _a + _b << std::endl;
  }
private:
  int  _no;
  T  _a, _b;
};
```

转账处理示例

```c++
#include <iostream>
#include <mutex>
#include <thread>

class Account
{
public:
  explicit Account( double balance ) : _balance(balance)  {  }
  double  GetBalance()  {  return _balance; }
  void  Increase( double amount )  {  _balance += amount;  }
  void  Decrease( double amount )  {  _balance -= amount;  }
  std::mutex &  GetMutex()  {  return _x; }
private:
  double  _balance;
  std::mutex  _x;
};

//  避免死锁，使用std::lock()函数锁定多个互斥，不同的锁定顺序不会导致死锁
//  加锁时有可能引发异常，std::lock()函数会处理该异常
//  将解锁此前已加锁的部分互斥，然后重新引发该异常
void Transfer( Account & from, Account & to, double amount )
{
  std::unique_lock<std::mutex>  locker1( from.GetMutex(), std::adopt_lock );
  std::unique_lock<std::mutex>  locker2( to.GetMutex(), std::adopt_lock );
  std::lock( from.GetMutex(), to.GetMutex() );
  from.Decrease( amount );    to.Increase( amount );
}

int main()
{
  Account  a1( 100.0 ), a2( 200.0 );
  //  线程参数采用值传递机制，如果要传递引用，调用std::ref()函数
  std::thread  t1( Transfer, std::ref( a1 ), std::ref( a2 ), 10.0 );
  std::thread  t2( Transfer, std::ref( a2 ), std::ref( a1 ), 20.0 );
  t1.join();    t2.join();
  return 0;
}
```



#### 条件变量类

`std::condition_variable` 类

- 必须与 `std::unique_lock` 配合使用

`std::condition_variable_any` 类

- 更加通用的条件变量，可以与任意型式的互斥锁配合使用，相比前者使用时会有额外的开销

多线程通信同步原语

- 阻塞一个或多个线程至收到来自其他线程的通知，超时或发生虚假唤醒
- 两者具有同样的成员函数，且在等待条件变量前都必须要获得相应的锁

成员函数 `notify_one()`：通知一个等待线程

- 原型：`void motify_one() noexcept;`

成员函数 `notify_all()`：通知全部等待线程

- 原型：`void motify_one() noexcept;`

成员函数 `wait()`：阻塞当前线程至被唤醒

- 原型：`template<typename Lock> void wait( Lock & lock );`

- 原型：`template<typename Lock, typename Predicate> void wait( Lock & lock, Predicate p );`

成员函数 `wait_for()`：阻塞至被唤醒或超过指定时长

- 原型： `template<typename Lock, typename Rep, typename _Period> cv_status wait_for( Lock& lock, const chrono::duration<Rep, Period>& rtime );`
- 原型：`template<typename Lock, typename Rep, typename Period, typename Predicate> bool wait_for( Lock& lock, const chrono::duration<Rep, Period>& rtime, Predicate p );`

成员函数 `wait_until()`：阻塞至被唤醒或到达指定时点

- 原型：`template<typename Lock, typename Clock, typename Duration> cv_status wait_until( Lock & lock, const chrono::time_point<Clock, Duration>& atime);`
- 原型：`template<typename Lock, typename Clock, typename Duration, typename Predicate> bool wait_until( Lock& lock, const chrono::time_point<Clock, Duration>& atime, Predicate p );`

```c++
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
std::mutex  x;
std::condition_variable  cond;
bool  ready = false;
bool  IsReady()  {  return ready;  }
void  Run( int no )
{
  std::unique_lock<std::mutex>  locker( x );
  while( !ready )		//  若标志位非true，阻塞当前线程
    cond.wait( locker );	//  解锁并睡眠，被唤醒后重新加锁
  //  以上两行代码等价于cond.wait( locker, &IsReady );
  //  第二个参数为谓词，亦可使用函子实现
  std::cout << "thread " << no << '\n';
}
int  main()
{
  std::thread  threads[8];
  for( int i = 0; i < 8; ++i )
    threads[i] = std::thread( Run, i );
  std::cout << "8 threads ready...\n";
  {
    std::unique_lock<std::mutex>  locker( x );    //  互斥加锁
    ready = true;		//  设置全局标志位为true
    cond.notify_all();	//  唤醒所有线程
  }    //  离开作用域，自动解锁；可将此复合语句块实现为函数
  //  基于区间的循环结构，对属于threads数组的所有元素t，执行循环体
  for( auto & t: threads )
    t.join();
  return 0;
}
```



#### 原子型式

使用方法

- 使用 `atomic` 模板定义原子对象
- 使用预定义标准原子型式：`atomic_bool`、`atomic_char`、`atomic_int`、`atomic_uint`、`atomic_long`、`atomic_wchar_t` 等等

意义：轻量级，支持单变量上的原子操作

| 操　作                                             | `atomic_ flag` | `atomic <bool>` | `atomic     <int_t>` | `atomic     <T *>` | `atomic     <other_t>` |
| -------------------------------------------------- | -------------- | --------------- | -------------------- | ------------------ | ---------------------- |
| `test_and_set`                                     | `√`            |                 |                      |                    |                        |
| `clear`                                            | √              |                 |                      |                    |                        |
| `is_lock_free`                                     |                | √               | √                    | √                  | √                      |
| `load`                                             |                | √               | √                    | √                  | √                      |
| `store`                                            |                | √               | √                    | √                  | √                      |
| `exchange`                                         |                | √               | √                    | √                  | √                      |
| `compare_exchange_weak`, `compare_exchange_strong` |                | √               | √                    | √                  | √                      |
| `fetch_add,` `+=`                                  |                |                 | √                    | √                  |                        |
| `fetch_sub,  -=`                                   |                |                 | √                    | √                  |                        |
| `fetch_or, |=`                                     |                |                 | √                    |                    |                        |
| `fetch_and,  &=`                                   |                |                 | √                    |                    |                        |
| `fetch_xor,  ^=`                                   |                |                 | √                    |                    |                        |
| `++, --`                                           |                |                 | √                    | √                  |                        |

```c++
#include <atomic>
#include <iostream>
#include <thread>
int  n = 0;
std::atomic<int>  a( 0 );
void  AddAtomically( int m )  {  while( m-- )  a.fetch_add( 1 );  }
void  Add( int m )  {  while( m-- )  ++n;  }
int  main()
{
  std::thread  ts1[8], ts2[8];
  for( auto & t: ts1 )  t = std::move( std::thread( AddAtomically, 1000000 ) );
  for( auto & t: ts2 )  t = std::move( std::thread( Add, 1000000 ) );
  for( auto & t: ts1 )  t.join();
  for( auto & t: ts2 )  t.join();
  //  输出结果：a值固定，而n值多次运行结果可能不同
  std::cout << "a = " << a << std::endl;
  std::cout << "n = " << n << std::endl;
  return 0;
}
```



#### 期许与承诺

线程返回值

- 为支持跨平台，`thread` 类无属性字段保存线程函数的返回值

解决方案

- 使用指针型式的函数参数
- 使用期许：`std::future` 类模板
- 使用承诺：`std::promise` 类模板

##### 指针型式参数

```c++
//  使用指针作为函数参数，获取线程计算结果
#include <iostream>
#include <vector>
#include <tuple>
#include <thread>
#include <mutex>
std::mutex x;
//  劳工线程类模板，处理T型数据对象
template< typename T >  class Worker
{
public:
  explicit Worker( int no, T a = 0, T b = 0 ) : _no(no), _a(a), _b(b)  {  }
  void  ThreadFunc( T * r )  {  x.lock();    *r = _a + _b;    x.unlock();  }
private:
  int  _no;	//  线程编号，非线程ID
  T  _a, _b;	//  保存在线程中的待处理数据
};

int main()
{
  //  定义能够存储8个三元组的向量v，元组首元素为指向劳工对象的指针
  //  次元素保存该劳工对象计算后的结果数据，尾元素为指向劳工线程对象的指针
  //  向量中的每个元素都表示一个描述线程运行的线程对象，
  //  该线程对象对应的执行具体任务的劳工对象，及该劳工对象运算后的返回值
  std::vector< std::tuple<Worker<int>*, int, std::thread*> >  v( 8 );

  //  构造三元组向量，三元编号顺次为0、1、2
  for( int i = 0; i < 8; i++ )
    v[i] = std::make_tuple( new Worker<int>( i, i+1, i+2 ), 0, nullptr );

  //  输出处理前结果；使用std::get<n>(v[i])获取向量的第i个元组的第n个元素
  //  三元编号顺次为0、1、2，因而1号元保存的将是劳工对象运算后的结果
  for( int i = 0; i < 8; i++ )
    std::cout << "No. " << i << ": result = " << std::get<1>(v[i]) << std::endl;
  //  创建8个线程分别计算
  for( int i = 0; i < 8; i++ )
  {
    //  将劳工类成员函数绑定为线程函数，对应劳工对象绑定为执行对象
    //  将构造线程对象时传递的附加参数作为被绑定的线程函数的第一个参数
    //  auto表示由编译器自动推断f的型式
    auto  f = std::bind( &Worker<int>::ThreadFunc,
      std::get<0>( v[i] ), std::placeholders::_1 );

    //  动态构造线程对象，并保存到向量的第i个三元组中
    //  传递三元组的1号元地址，即将该地址作为线程函数的参数
    //  线程将在执行时将结果写入该地址
    //  此性质由绑定函数std::bind()使用占位符std::placeholders::_1指定
    //  线程对象为2号元，即三元组的最后一个元素
    std::get<2>( v[i] ) = new std::thread( f, &std::get<1>( v[i] ) );
  }

  for( int i = 0; i < 8; i++ )
  {
    //  等待线程结束
    std::get<2>( v[i] )->join(); 
    //  销毁劳工对象
    delete std::get<0>( v[i] ),    std::get<0>( v[i] ) = nullptr;
    //  销毁线程对象
    delete std::get<2>( v[i] ),    std::get<2>( v[i] ) = nullptr;
  }
  //  输出线程计算后的结果
  for( int i = 0; i < 8; i++ )
    std::cout << "No. " << i << ": result = " << std::get<1>(v[i]) << std::endl;
  return 0;
}
```

##### 期许

`std::future` 类模板

- 目的：获取异步操作结果，延迟引发线程的异步操作异常

使用方法

- 定义期许模板类的期许对象
- 使用 `std::async()` 函数的返回值初始化
- 调用期许对象的成员函数 `get()` 获取线程返回值

```c++
//  使用期许对象获取线程返回值
#include <iostream>
#include <exception>
#include <thread>
#include <future>

unsigned long int  CalculateFactorial( short int n )
{
  unsigned long int  r = 1;
  if( n > 20 )
    throw std::range_error( "The number is too big." );
  for( short int i = 2; i <= n; i++ )
    r *= i;
  return r;
}

int main()
{
  short int  n = 20;
  //  启动异步线程，执行后台计算任务，并返回std::future对象
  std::future<unsigned long int>  f = std::async( CalculateFactorial, n );
  try  {
    //  获取线程返回值，若线程已结束，立即返回，否则等待该线程计算完毕
    //  若线程引发异常，则延迟到std::future::get()或std::future::wait()调用时引发
    unsigned long int  r = f.get();
    std::cout << n << "! = " << r << std::endl;
  }
  catch( const std::range_error & e )  {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
```



##### 承诺

`std::promise` 类模板

- 目的： 承诺对象允许期许对象获取线程对象创建的线程返回值

使用方法

- 创建承诺 `std::promise<T>` 对象
- 获取该承诺对象的相关期许 `std::future<T>` 对象
- 创建线程对象，并传递承诺对象
- 线程函数内部通过承诺模板类的成员函数`set_value()`、`set_value_at_thread_exit()`、`set_exception()`或`set_exception_at_thread_exit()` 设置值或异常
- 通过期许对象等待并获取异步操作结果

```c++
//  使用承诺对象设置线程返回值
#include <iostream>
#include <exception>
#include <thread>
#include <future>

unsigned long int CalculateFactorial( short int n )
{
  unsigned long int r = 1;
  if( n > 20 )    throw std::range_error( "The number is too big." );
  for( short int i = 2; i <= n; i++ )    r *= i;
  return r;
}

//  CalculateFactorial()函数的包装函数原型
void DoCalculateFactorial(
  std::promise<unsigned long int> && promise, short int n );

int main()
{
  short int  n = 6;
  std::promise<unsigned long int>  p;    //  创建承诺对象
  std::future<unsigned long int>  f = p.get_future();    //  获取相关期许对象
  //  启动线程，执行CalculateFactorial()函数的包装函数
  std::thread  t( DoCalculateFactorial, std::move( p ), n );
  t.detach();
  try
  {
    unsigned long int  r = f.get();
    std::cout << n << "! = " << r << std::endl;
  }
  catch( std::range_error & e )
    std::cerr << e.what() << std::endl;
  return 0;
}

//  CalculateFactorial()函数的包装函数实现
void DoCalculateFactorial(
  std::promise<unsigned long int> && promise, short int n )
{
  try
  {
    //  设置线程返回值，供期许对象获取
    promise.set_value( CalculateFactorial( n ) );
  }
  catch( ... )
  {
    //  捕获全部异常，并在期许获取线程返回值时重新引发
    promise.set_exception( std::current_exception() );
  }
}
```



14.1 考虑作业队列的容量限制，修改代码，实现标准的生产者—消费者模型。设作业队列最多容量amount个作业，有m个接收作业的线程，有n个处理作业的线程。

14.2 实现Linux互斥、信号量和条件变量的封装类，并使用上述同步机制实现线程池类。提示：（1）线程池功能与实现类似于进程池。（2）以作业型作为模板形式参数实现作业处理线程池类和作业处理类。（3）线程函数为静态函数，要访问类的非静态成员，可以定义类的静态对象或者传递对象指针，在线程函数中通过该对象指针访问其成员。（4）可以参考C++11架构。

