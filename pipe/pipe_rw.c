#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define MAXLINE 100

int
test(void)
{
    int     n;
    int     fd[2];
    pid_t   pid;
    char    line[MAXLINE];

    if (pipe(fd) < 0)
        printf("pipe error");
    if ((pid = fork()) < 0) {
        printf("fork error");
    } else if (pid > 0) {       /* parent */
        const char *input = "hello world\n";
        close(fd[0]);
        write(fd[1], input, strlen(input));
    } else {                /* child */
        close(fd[1]);
        n = read(fd[0], line, MAXLINE);
        write(STDOUT_FILENO, line, n);
    }
    //exit(0);
}

int
test2(void)
{
    int     n;
    int     fd[2];
    pid_t   pid;
    char    line[MAXLINE];

    if (pipe(fd) < 0)
        printf("pipe error");
    if ((pid = fork()) < 0) {
        printf("fork error");
    } else if (pid > 0) {       /* parent */
        const char *input = "hello world\n";
        close(fd[0]);
        write(fd[1], input, strlen(input));
    } else {                /* child */
        close(fd[1]);
        n = read(fd[0], line, MAXLINE);
        write(STDOUT_FILENO, line, n);
    }
    //exit(0);
}


int main(){
    test();
    test2();

    return 1;
}