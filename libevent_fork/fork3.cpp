#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <event.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "./atomic.h"

#define ForkNum 10
#define SERVER_PORT 7000
#define FifoServer "/tmp/fifo.server."
#define FifoClient "/tmp/fifo.client."
//#define FifoNum 2000

//FIFO FD
int FIFO_CLIENT_R = -1;
int FIFO_CLIENT_W = -1;
int FIFO_SERVER_R = -1;
int FIFO_SERVER_W = -1;
//FIFO FD
int ForkPos=0;
atomic_t ActiveConnectionAtomic;
static int ActiveConnection = 0;

void fd2client_fifopath(int fd,char *path){
	int pos = fd % ForkNum;
	sprintf( path, "%s%d", FifoClient,pos);
}
void fd2server_fifopath(int fd,char *path){
	int pos = fd % ForkNum;
	sprintf( path, "%s%d", FifoServer,pos);
}

void init_fifo(){
	char ClientFifoPath[100];
	char ServerFifoPath[100];
    // if(mkfifo(FifoServer,O_CREAT|O_EXCL)){
    //    //printf("cannot create fifoserver\n");
    // }

	for(int i=0;i<ForkNum;i++){
		fd2client_fifopath(i,ClientFifoPath);
		fd2server_fifopath(i,ServerFifoPath);
		//sprintf( ClientFifoPath, "%s%d", FifoClient,i);

		//write 2 
	    if(mkfifo(ClientFifoPath,O_CREAT|O_EXCL)){
	       //printf("cannot create fifoserver\n");
	    }		
	    if(mkfifo(ServerFifoPath,O_CREAT|O_EXCL)){
	       //printf("cannot create fifoserver\n");
	    }		
	}
}
void client_writefifo(int fd){
	int len = 100;
	int nwrite = 0;
	char sfd[len];
	char ClientFifoPath[100];
	sprintf( sfd, "%d", fd);
	fd2client_fifopath(ForkPos,ClientFifoPath);
	printf("%s\n", ClientFifoPath);

	// if((ForkPos == 1) || (ForkPos == 2)){
	// 	sleep(1);
	// }
	if(FIFO_CLIENT_W == -1){
		FIFO_CLIENT_W = open(ClientFifoPath,O_WRONLY,0);
	}

	//int fifo_fd=open(ClientFifoPath,O_WRONLY,0);
	if((nwrite=write(FIFO_CLIENT_W,sfd,len))==-1)
	{
		if(errno==EAGAIN){
			printf("The FIFO has not been read yet.Please try later\n");
		}else{
			printf("fifo.error:%d\n",errno);
		}
	}
	else{
		printf("client.fifo.write:%s\n",sfd);
	}

}

void client_readfifo(){
	int len = 100;
	int nread = 0;
	char sfd[len];
	char ClientFifoPath[100];
	char ServerFifoPath[100];
	char buf_r[len];
	//sprintf( sfd, "%d", fd);
	//sprintf( ClientFifoPath, "%s%d", FifoClient,fd);
	fd2server_fifopath(ForkPos,ServerFifoPath);

	if(FIFO_SERVER_R == -1){
		FIFO_SERVER_R=open(ServerFifoPath,O_RDONLY,0);
	}	
	if((nread=read(FIFO_SERVER_R,buf_r,len))==-1){
        if(errno==EAGAIN){
            //printf("no data yet\n");
        }
	}else{
		printf("server.fifo.read:%s\n",buf_r);
		int client_fd = atoi(buf_r);
		//client_fd = client_fd * 10;
		client_writefifo(client_fd);
	}
}

void server_readfifo(int fd){
	int len = 100;
	int nread = 0;
	char sfd[len];
	char ClientFifoPath[100];
	char buf_r[len];
	//sprintf( sfd, "%d", fd);
	fd2client_fifopath(fd,ClientFifoPath);
	if(FIFO_CLIENT_R == -1){
		FIFO_CLIENT_R=open(ClientFifoPath,O_RDONLY,0);
	}
	
	if((nread=read(FIFO_CLIENT_R,buf_r,len))==-1){
            if(errno==EAGAIN){
                //printf("no data yet\n");
            }
	}else{
		printf("client.fifo.read:%s\n",buf_r);
	}
	//close(fifo_fd);

}

void server_writefifo(int fd){
	int len = 100;
	int nwrite = 0;
	char sfd[len];
	char ClientFifoPath[100];
	char ServerFifoPath[100];
	sprintf( sfd, "%d", fd);
	fd2server_fifopath(fd,ServerFifoPath);

	//write 2 
    // if(mkfifo(FifoServer,O_CREAT|O_EXCL)){
    //    //printf("cannot create fifoserver\n");
    // }
    // if(mkfifo(ClientFifoPath,O_CREAT|O_EXCL)){
    //    //printf("cannot create fifoserver\n");
    // }

	if(FIFO_SERVER_W == -1){
		FIFO_SERVER_W=open(ServerFifoPath,O_WRONLY,0);
	}
	
	if((nwrite=write(FIFO_SERVER_W,sfd,len))==-1)
	{
		if(errno==EAGAIN){
			printf("The FIFO has not been read yet.Please try later\n");
		}
	}
	else{
		printf("server.fifo.write:%s\n",sfd);
	}
	server_readfifo(fd);
}

void on_write(int fd, short event, void *arg1)
{
	char buf[32];
	struct tm t;
	time_t now;
	struct event *arg = (struct event *)arg1;

	time(&now);
	localtime_r(&now, &t);
	asctime_r(&t, buf);

//	memset(buf,0,strlen(buf));
	read(fd,buf,strlen(buf));
	server_writefifo(fd); //逻辑处理

	write(fd, buf, strlen(buf));
	close(fd);
	//ActiveConnection--;	
	atomic_dec(&ActiveConnectionAtomic);
	//shutdown(fd, SHUT_RDWR);

	free(arg1);
}

void connection_accept(int fd, short event, void *arg)
{
	//ActiveConnection++;
	atomic_add(1,&ActiveConnectionAtomic);
	//printf("%s\n", "ha");
	/* for debugging */
	ActiveConnection=atomic_read(&ActiveConnectionAtomic);
	if(ActiveConnection > 1000){
		return;
	}
	fprintf(stderr, "%s(): fd = %d, event = %d, conn = %d.\n", __func__, fd, event,ActiveConnection);

	/* Accept a new connection. */
	struct sockaddr_in s_in;
	socklen_t len = sizeof(s_in);
	int ns = accept(fd, (struct sockaddr *) &s_in, &len);
	if (ns < 0) {
		perror("accept->");
		return;
	}
	

	/* Install time server. */
	//struct event *ev = (struct event *)arg;
	struct event *ev = (struct event *)malloc(sizeof(struct event));
	event_set(ev, ns, EV_WRITE, on_write, ev);
	event_add(ev, NULL);
}



int net_process(void)
{
	int pid = fork();
	if(pid ==0 ){
		return 0;
	}

	/* Request socket. */
	int s = socket(PF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		perror("socket");
		exit(1);
	}
	int opt = 1;
	setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

	/* bind() */
	struct sockaddr_in s_in;
	bzero(&s_in, sizeof(s_in));
	s_in.sin_family = AF_INET;
	s_in.sin_port = htons(SERVER_PORT);
	s_in.sin_addr.s_addr = INADDR_ANY;
	if (bind(s, (struct sockaddr *) &s_in, sizeof(s_in)) < 0) {
		perror("bind");
		exit(1);
	}

	/* listen() */
	if (listen(s, 500) < 0) {
		perror("listen");
		exit(1);
	}

	/* Initial libevent. */
	event_init();

	/* Create event. */
	struct event ev;
	//event_set(&ev, s, EV_READ | EV_PERSIST, on_read, &ev);
	event_set(&ev, s, EV_WRITE | EV_PERSIST, connection_accept, &ev);

	/* Add event. */
	event_add(&ev, NULL);

	event_dispatch();

	return 0;
}

int child_process() {
	int pid = getpid();
	int filePos = pid % ForkNum;

    while (1) {
    	client_readfifo();
    	//usleep(1000000);
    	usleep(10);
    	printf("%s-%d-pos:%d\n", "child",pid,ForkPos);
    }
    printf("%s-%d\n", "child_process",pid);
}

int fork_process(int num){
	int i,k;
	int pid = -1;
	for(i=0;i<num;i++){
		if(pid != 0){
			ForkPos++;
			pid = fork();
			printf(":::%d\n", pid);
			if(pid == 0){
				child_process();
				break;
			}else{
				//waitpid(pid,NULL,0);
			}
		}else{

			//printf("This is parent process[%i]\n",pid);
		}
	}
	return pid;
}

int main(){
	//signal(SIGCLD, SIG_IGN);

//printf("%s\n", eventop.);
	atomic_set(&ActiveConnectionAtomic,0);
	init_fifo();
    fork_process(ForkNum);
	net_process();

	return 1;
}
