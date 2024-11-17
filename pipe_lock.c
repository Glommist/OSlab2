#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
int pid1=-1,pid2=-1; // 定义两个进程变量
int main( )
{
    int fd[2];
    char InPipe[1000]; // 定义读缓冲区
    char* OutPipe;
    while((pipe(fd)) == -1); // 创建管道
    while((pid1 = fork( )) == -1){} // 如果进程 1 创建不成功,则空循环
    if(pid1 > 0)
    {
        while ((pid2 = fork()) == -1){}
        if(pid2 > 0)
        {
            // 等待子进程 1 结束
            // 等待子进程 2 结束
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
            ssize_t bytes_read = read(fd[0], InPipe, sizeof(InPipe) - 1);
            // 加字符串结束符
            InPipe[bytes_read] = '\0';
            printf("%s\n",InPipe); // 显示读出的数据
            exit(0); // 父进程结束
        }
        else
        {
            lockf(fd[1],1,0);
            // 分 2000 次每次向管道写入字符’2’
            OutPipe = "Child process 2 is sending message!";
            write(fd[1],OutPipe,strlen(OutPipe));
            sleep(1);
            lockf(fd[1],0,0);
            exit(0);
        }
    }
    else
    {
        // 如果子进程 1 创建成功,pid1 为进程号
        // 锁定管道
        lockf(fd[1],1,0);
        // 分 2000 次每次向管道写入字符’1’
        OutPipe = "Child process 1 is sending message!";
        write(fd[1],OutPipe,strlen(OutPipe));
        // 等待读进程读出数据
        sleep(1);
        // 解除管道的锁定
        lockf(fd[1],0,0);
        exit(0); // 结束进程 1
    }
}
