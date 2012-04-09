#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char**argv)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct hostent *server = gethostbyname("127.0.0.1");
    if (server == NULL) {
        printf("ERROR, no such host\n");
        exit(0);
    }

    struct sockaddr_in serv_addr;
    memset((char*) &serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    memcpy((char*)server->h_addr,
            (char*)&serv_addr.sin_addr.s_addr,
            server->h_length);
    serv_addr.sin_port = htons(9995);
    if (connect( sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        printf("error connect:%d\n", errno);
        exit(0);
    }
    
    const char *line = "aaa";
    char buf[1024]= {0};
    for(int i=0;i<2;i++){
        write(sockfd,line,strlen(line));

        int n = read( sockfd, buf,sizeof(buf));
        buf[n]='\0';
        printf("%s\n", buf);
            //sleep(10);
    }

    close(sockfd);
    return 0;
}