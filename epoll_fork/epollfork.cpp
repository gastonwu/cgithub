#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdlib.h>
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
    //  sleep(1);
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

void on_write(int fd)
{
    char buf[32];
    struct tm t;
    time_t now;
    //struct event *arg = (struct event *)arg1;

    time(&now);
    localtime_r(&now, &t);
    asctime_r(&t, buf);

//  memset(buf,0,strlen(buf));
    read(fd,buf,strlen(buf));
    server_writefifo(fd); //逻辑处理

    write(fd, buf, strlen(buf));
    close(fd);
    //ActiveConnection--;   
    atomic_dec(&ActiveConnectionAtomic);
    //shutdown(fd, SHUT_RDWR);

    //free(arg1);
}

// void connection_accept(int fd, short event, void *arg)
// {
//     //ActiveConnection++;
//     atomic_add(1,&ActiveConnectionAtomic);
//     //printf("%s\n", "ha");
//     /* for debugging */
//     ActiveConnection=atomic_read(&ActiveConnectionAtomic);
//     if(ActiveConnection > 1000){
//         return;
//     }
//     fprintf(stderr, "%s(): fd = %d, event = %d, conn = %d.\n", __func__, fd, event,ActiveConnection);

//     /* Accept a new connection. */
//     struct sockaddr_in s_in;
//     socklen_t len = sizeof(s_in);
//     int ns = accept(fd, (struct sockaddr *) &s_in, &len);
//     if (ns < 0) {
//         perror("accept->");
//         return;
//     }
    

//     /* Install time server. */
//     //struct event *ev = (struct event *)arg;
//     struct event *ev = (struct event *)malloc(sizeof(struct event));
//     event_set(ev, ns, EV_WRITE, on_write, ev);
//     event_add(ev, NULL);
// }




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


int net_process() {
    int listen_fd, accept_fd, flag;
    struct sockaddr_in my_addr, remote_addr;
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("create socket error");
        exit(1);
    }
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR
            , (char *) &flag, sizeof (flag)) == -1) {
        perror("setsockopt error");
    }
    int flags = fcntl(listen_fd, F_GETFL, 0);
    fcntl(listen_fd, F_SETFL, flags | O_NONBLOCK);
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(SERVER_PORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(my_addr.sin_zero), 8);
    if (bind(listen_fd, (struct sockaddr *) &my_addr,
            sizeof (struct sockaddr_in)) == -1) {
        perror("bind error");
        exit(1);
    }
    if (listen(listen_fd, 1) == -1) {
        perror("listen error");
        exit(1);
    }
    int pid = -1;
    int addr_len = sizeof (struct sockaddr_in);
    int max_process_num = 3;
    int child_id;
    int i;
    int child_process_status;
    for (i = 0; i < max_process_num; i++) {
        if (pid != 0) {
            pid = fork();
        } else {
            child_id = i;
        }
    }


    if (pid == 0) {
        int accept_handles = 0;
        struct epoll_event ev, events[20];
        int epfd = epoll_create(256);
        int ev_s = 0;
        ev.data.fd = listen_fd;
        ev.events = EPOLLIN | EPOLLET;
        epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);
        for (;;) {
            ev_s = epoll_wait(epfd, events, 20, 500);
            int i = 0;
            for (i = 0; i < ev_s; i++) {
                if (events[i].data.fd == listen_fd) {
                    int max_process_accept = 3;
                    if (accept_handles < max_process_accept) {
                        accept_handles++;

    // struct sockaddr_in s_in;
    // socklen_t len = sizeof(s_in);
    // int ns = accept(fd, (struct sockaddr *) &s_in, &len);


                        //int addr_len = sizeof ( struct sockaddr_in);
                        struct sockaddr_in s_in;
                        socklen_t addr_len = sizeof(s_in);

                        accept_fd = accept(listen_fd,(struct sockaddr *) &remote_addr, &addr_len);


                        int flags = fcntl(accept_fd, F_GETFL, 0);
                        fcntl(accept_fd, F_SETFL, flags | O_NONBLOCK);
                        ev.data.fd = accept_fd;
                        ev.events = EPOLLIN | EPOLLET;
                        epoll_ctl(epfd, EPOLL_CTL_ADD, accept_fd, &ev);
                        printf("1->Child:%d,ProcessID:%d,EPOLLIN,fd:%d,accept:%d\n", child_id, getpid(), listen_fd, accept_fd);
                    }
                } else if (events[i].events & EPOLLIN) {
                    char in_buf[1024];
                    memset(in_buf, 0, 1024);
                    int recv_num = recv(events[i].data.fd, &in_buf, 1024, 0);
                    if (recv_num == 0) {
                        close(events[i].data.fd);
                        accept_handles--;
                        printf("2->Child:%d,ProcessID:%d,EPOLLIN,fd:%d,closed\n", child_id, getpid(), events[i].data.fd);
                    } else {
			//child_write_fifo(data.fd);
                        printf("3->Child:%d,ProcessID:%d,EPOLLIN,fd:%d,recv:%s\n", child_id, getpid(), events[i].data.fd, in_buf);
                        char hello[] = "Hello! (通讯)Socket communication world!\n";
                        if (send(events[i].data.fd, hello, strlen(hello), 0) == -1) {
                            fprintf(stderr, "Write Error:%s\n", strerror(errno));
                            exit(1);
                        }
                        close(events[i].data.fd);
                        accept_handles--;
                    }
                }
            }
        }
    } else {
        //manager the process
        wait(&child_process_status);
    }

    return 0;
}


int main(){
    signal(SIGCLD, SIG_IGN);

//printf("%s\n", eventop.);
    atomic_set(&ActiveConnectionAtomic,0);
    init_fifo();
    fork_process(ForkNum);
    net_process();

    return 1;
}