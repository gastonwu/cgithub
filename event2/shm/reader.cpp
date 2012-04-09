#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
int main(void)
{
    int shmid;
    char *addr;
    shmid = shmget(12345, 1024, IPC_CREAT | 0600);
    if (shmid == -1)
    {
        perror("shmget");
        return -1;
    }
    addr = (char *)shmat(shmid, NULL, 0);
    if (addr == (void *)-1)
    {
        perror("shmat");
        return -1;
    }
    printf("addr = %s\n", addr);
    shmdt(addr);
    return 0;
}

