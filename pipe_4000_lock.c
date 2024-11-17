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
    char InPipe[5000]; // 定义读缓冲区
    char c1='1', c2='2';
    while((pipe(fd)) == -1); // 创建管道
    while((pid1 = fork( )) == -1){} // 如果进程 1 创建不成功,则空循环
    if(pid1 > 0)
    {
        while((pid2 = fork()) == -1){}
        if(pid2 > 0)
        {
            // 等待子进程 1 结束
            // 等待子进程 2 结束
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
            // 从管道中读出 4000 个字符
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
            for(int i = 0; i < 2000 ;i++)
                write(fd[1],&c2,1);
            sleep(5);
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
        for(int i = 0; i < 2000 ;i++)
            write(fd[1],&c1,1);
        // 等待读进程读出数据
        sleep(5);
        // 解除管道的锁定
        lockf(fd[1],0,0);
        exit(0); // 结束进程 1
    }
}