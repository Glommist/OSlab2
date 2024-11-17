#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>

int flag = 0;
pid_t pid1 = -1, pid2 = -1;

void inter_handler(int sig) 
{
    if(sig == 3){printf("\n3 stop test\n");}
    if(sig == 2){printf("\n2 stop test\n");}
    kill(pid1, 16);
    kill(pid2, 17);
    sleep(1);
    flag = 1;
}
void inter_handler_child(int sig) 
{
    if(sig == 16){printf("\n16 stop test\n");}
    if(sig == 17){printf("\n17 stop test\n");}
}
void waiting()
{
   int temp = 0;
   while(flag == 0&& temp < 5)
   {
        sleep(1);
        temp++;
   }
}
int main()
{
    while (pid1 == -1) pid1 = fork();
    if (pid1 > 0)
    {
        while (pid2 == -1) pid2 = fork();
        if (pid2 > 0)
        {
            // 父进程
            signal(SIGINT, inter_handler);
            signal(SIGQUIT, inter_handler);
            waiting();
            kill(pid1, SIGALRM);
            kill(pid2, SIGALRM);
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
            printf("\nParent process is killed!!\n");
        }
        else 
        {
            // 子进程 2
            signal(SIGINT, SIG_IGN);
            signal(SIGQUIT, SIG_IGN);
            signal(17, inter_handler_child);
            while(pause() == -1)
            {
                printf("\nChild process 2 is killed by parent!!\n");
            }
            exit(0);
        }
    } 
    else
    {
        // 子进程 1
        signal(SIGINT, SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
        signal(16, inter_handler_child);
        while(pause() == -1)
        {
            printf("\nChild process 1 is killed by parent!!\n");
        }
        exit(0);
    }
    return 0;
}
