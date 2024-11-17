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
            while((pid2 = fork()) == -1){}
            if(pid2 > 0)
            {
                waitpid(pid1, NULL, 0);
                waitpid(pid2, NULL, 0);
                ssize_t bytes_read = read(fd[0], InPipe, sizeof(InPipe) - 1);
                InPipe[bytes_read] = '\0';
                printf("%s\n",InPipe); // 显示读出的数据
                exit(0); // 父进程结束
            }
            else
            {
                OutPipe = "Child process 2 is sending message!";
                write(fd[1],OutPipe,strlen(OutPipe));
                sleep(1);
                exit(0);
            }
        }
        else
        {
            // 如果子进程 1 创建成功,pid1 为进程号
            OutPipe = "Child process 1 is sending message!";
            write(fd[1],OutPipe,strlen(OutPipe));
            sleep(1);
            exit(0);
        }
}
