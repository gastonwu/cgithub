/***** testwrite.c *******/
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
	char name[4];
	int age;
} people;

int write1()
{
	int shm_id,i;
	key_t key;
	char temp;
	people *p_map;
	const char* name = "/tmp/myshm2";
	key = ftok(name,0);
	if(key==-1)
		perror("ftok error2");
	shm_id=shmget(key,4096,IPC_CREAT);	
	if(shm_id==-1)
	{
		perror("shmget error");
		return 1;
	}
	p_map=(people*)shmat(shm_id,NULL,0);
	temp='a';
	for(i = 0;i<10;i++)
	{
		temp+=1;
		memcpy((*(p_map+i)).name,&temp,1);
		(*(p_map+i)).age=20+i;
	}
	if(shmdt(p_map)==-1)
		perror(" detach error ");
}

int write2()
{
	int shm_id,i;
	key_t key;
	char temp;
	people *p_map;
	//char line[100];

	const char* name = "/tmp/myshm3";
	key = ftok(name,0);
	if(key==-1){
		//perror("ftok error2");
	}
	shm_id=shmget(key,4096,IPC_CREAT);	
	if(shm_id==-1)
	{
		perror("shmget error");
		return 1;
	}
	//p_map=(people*)shmat(shm_id,NULL,0);
	char* line=(char *)shmat(shm_id,NULL,0);

	sprintf(line," line\0");

	if(shmdt(p_map)==-1)
		perror(" detach error ");
}

int read2()
{
	int shm_id,i;
	key_t key;
	people *p_map;
	char buf[200];
	const char* name = "/tmp/myshm3";
	key = ftok(name,0);
	if(key == -1)
		perror("ftok error");
	shm_id = shmget(key,4096,IPC_CREAT);	
	if(shm_id == -1)
	{
		perror("shmget error");
		return 1;
	}
	char* line= (char *)shmat(shm_id,NULL,0);
	strcpy(buf, line);
	sprintf("::%s\n",buf);

	
	if(shmdt(p_map) == -1)
		perror(" detach error ");
}
int main(){
	write1();
	write2();
	read2();


}
