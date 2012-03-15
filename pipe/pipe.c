#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
 
void sig_pipe(int signo)
{
     write(STDOUT_FILENO, "receive SIGPIPE\n", 17);
}
 
int main()
{
     int fd[2];
     pid_t pid;
     char buf[10];
 
     memset(buf, 0, 10);
     if(signal(SIGPIPE, sig_pipe) == SIG_ERR)
       perror("signal SIGPIPE error");
 
     if(pipe(fd) < 0)
     {
       perror("pipe error");
       exit(EXIT_FAILURE);
     }
    
     if((pid = fork()) < 0)
     {
       perror("fork error");
       exit(EXIT_FAILURE);
     }
     else if(pid == 0)
     {
       close(fd[1]);
       exit(EXIT_SUCCESS);
     }
     close(fd[0]);
     sleep(1);
     if(write(fd[1], buf, 10) == -1)
       perror("write error");
     exit(EXIT_SUCCESS);
}