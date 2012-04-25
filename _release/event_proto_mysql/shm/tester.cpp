#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int writer(int key,const char* line)
{
    int shmid;
    char *addr;
    shmid = shmget(key, 1024, IPC_CREAT | 0600);
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
    strcpy(addr, line);
    shmdt(addr);
    return 0;
}


int reader(int key,char* line)
{
    int shmid;
    char *addr;
    shmid = shmget(key, 1024, IPC_CREAT | 0600);
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
    sprintf(line,"%s",addr);
    //printf("addr = %s\n", addr);
    shmdt(addr);
    return 0;
}

int main(){
	int key = 123;
	char line[100];
	writer(key,"aaa");
	reader(key,line);
	printf("line:%s\n",line);

}
