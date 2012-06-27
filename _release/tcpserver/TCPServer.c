/*******服务器程序  TCPServer.c ************/
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <unistd.h>

#include <arpa/inet.h>


#define WAITBUF 10

int main(int argc, char *argv[]) {
    int sockfd, new_fd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int portnumber;
    socklen_t sin_size;
    char hello[] = "Hello! (通讯)Socket communication world!\n";

    if (argc != 2) {
        fprintf(stderr, "Usage:%s portnumber\a\n", argv[0]);
        exit(1);
    }
    /*端口号不对，退出*/
    if ((portnumber = atoi(argv[1])) < 0) {
        fprintf(stderr, "Usage:%s portnumber\a\n", argv[0]);
        exit(1);
    }

    /*服务器端开始建立socket描述符*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Socket error:%s\n\a", strerror(errno));
        exit(1);
    }

    /*服务器端填充 sockaddr结构*/
    bzero(&server_addr, sizeof (struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    /*自动填充主机IP*/
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(portnumber);

    /*捆绑sockfd描述符*/
    if (bind(sockfd, (struct sockaddr *) (&server_addr), sizeof (struct sockaddr)) == -1) {
        fprintf(stderr, "Bind error:%s\n\a", strerror(errno));
        exit(1);
    }

    /*监听sockfd描述符*/
    if (listen(sockfd, WAITBUF) == -1) {
        fprintf(stderr, "Listen error:%s\n\a", strerror(errno));
        exit(1);
    }
    
    int recvMsgMax = 1000;
    char recvMsgLine[recvMsgMax];
    int n;

    while (1) {
        /*服务器阻塞，直到客户程序建立连接*/
        sin_size = sizeof (struct sockaddr_in);
        if ((new_fd = accept(sockfd, (struct sockaddr *) (&client_addr), &sin_size)) == -1) {
            fprintf(stderr, "Accept error:%s\n\a", strerror(errno));
            exit(1);
        }
        /*可以在这里加上自己的处理函数*/
        fprintf(stderr, "Server get connection from %s\n",
                inet_ntoa(client_addr.sin_addr));
        n = recvfrom(sockfd, recvMsgLine, recvMsgMax, 0, (struct sockaddr*) &client_addr, &sin_size);
        recvMsgLine[n] = 0;
        fprintf(stdout, "::%d - %s\n", n, recvMsgLine);


        if (send(new_fd, hello, strlen(hello), 0) == -1) {
            fprintf(stderr, "Write Error:%s\n", strerror(errno));
            exit(1);
        }
        /*这个通信已经结束*/
        close(new_fd);
        /*循环下一个*/
    }
    close(sockfd);
    exit(0);
}
