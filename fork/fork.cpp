#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
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
#include <sys/ipc.h>
#include <sys/shm.h>

//mysql
#include <string.h>
#include <string>
#include <map>
#include <vector>
#include <mysql.h>

//proto buffer
#include "sqlparam.pb.h"
#include "result.pb.h"
#include <iostream>
#include <fstream>

#define FIFO "/tmp/myfifo"
#define SHM_START 10000;
#define SHM_SQL_START 20000;

using namespace std;

char proxy_mysql_errmsg[1024];
MYSQL proxy_mysql_connection;
MYSQL_RES* proxy_mysql_result;
MYSQL_ROW proxy_mysql_row;
bool proxy_mysql_is_connection;

int debug(const char* line) {
    printf("%s\n", line);
    return 0;
}

int proxy_mysql_connect() {
    //*
    if (!proxy_mysql_is_connection) {
        if (mysql_real_connect(&proxy_mysql_connection, "127.0.0.1", "root", "windows", "", 3306, NULL, 0) == NULL) {
            sprintf(proxy_mysql_errmsg, "Connecting to DataBase error:%s", mysql_error(&proxy_mysql_connection));
            return -1;
        }

        //it is better to change the config
        if (mysql_options(&proxy_mysql_connection, MYSQL_SET_CHARSET_NAME, "UTF8")) {
            sprintf(proxy_mysql_errmsg, "set charset names error:%s", mysql_error(&proxy_mysql_connection));
            return -1;

        }
        if (mysql_set_character_set(&proxy_mysql_connection, "UTF8")) {
            sprintf(proxy_mysql_errmsg, "set character error:%s", mysql_error(&proxy_mysql_connection));
            return -1;
        }
        proxy_mysql_is_connection = true;
        printf("connect mysql success.\n");
    } else {

    }
    //*/
    return 0;
}

string sqlBind(dbproxy::sqlparam sqlparam) {

    string sql2 = sqlparam.sql();
    int pos = 0;
    string pos_str;
    for (int i = 0; i < sql2.length(); i++) {
        if (sql2[i] != '%') {
            continue;
        }
        if ((i + 1) >= sql2.length()) {
            continue;
        }
        if (sql2[i + 1] != 's') {
            continue;
        }
        sql2.replace(i, 2, sqlparam.param(pos));
        string pos_str = sqlparam.param(pos);
        i = i + pos_str.length();
        pos++;


    }

    return sql2;
}

dbproxy::dbresult proxy_mysql_query(string sql) {
    MYSQL *conn;
    MYSQL_RES *result;
    MYSQL_ROW row;
    MYSQL_FIELD *field;
    int num_fields, i;


    proxy_mysql_connect();
    //sql = "select * from t1";
    cout << "sql:" + sql + "\n" << endl;

    mysql_query(&proxy_mysql_connection, sql.c_str());
    int affectRow = mysql_affected_rows(&proxy_mysql_connection);


    result = mysql_store_result(&proxy_mysql_connection);

    if (result) {
        num_fields = mysql_num_fields(result);
    } else {
        num_fields = 0;
    }



    map<int, string> mapField;

    int loop = 0;
    if (result) {
        while (field = mysql_fetch_field(result)) {
            mapField[loop] = field->name;
            loop++;
            printf("%s ", field->name);
        }
    }

    dbproxy::dbresult dbresult;
    {

    }
    dbresult.set_affectrows(affectRow);



    if (result) {
        while ((row = mysql_fetch_row(result))) {
            dbproxy::dbrow* dbrow = dbresult.add_rows();
            for (i = 0; i < num_fields; i++) {
                dbproxy::dbcol* dbcol = dbrow->add_cols();
                dbcol->set_field(mapField[i]);
                dbcol->set_val(row[i]);

            }
        }
    }

    mysql_free_result(result);


    return dbresult;
}

int shm_writer(int key,const char* line)
{
    int shmid;
    char *addr;
    shmid = shmget(key, 10240, IPC_CREAT | 0600);
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


int shm_reader(int key,char* line)
{
    int shmid;
    char *addr;
    shmid = shmget(key, 10240, IPC_CREAT | 0600);
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

int fifowrite(int pid,int fifofile=0) {
    int fd;
    char w_buf[100];
    int nwrite;
    char fifo_file[100];
    
    if(fifofile == 0){
        sprintf(fifo_file,"%s",FIFO);
    }else{
        sprintf(fifo_file,"%s.%d",FIFO,fifofile);
    }
    printf("write:::::%s\n",fifo_file);

    debug("block write start");
    if (mkfifo(fifo_file, O_CREAT | O_EXCL)) {
        debug("fifo created before");
    } else {
        debug("fifo create ok");
    }
    char line[10];
    sprintf(line,"%d",pid);
    //const char *line = "abc";
    //while (1) {
        //usleep(1000 * 500);
        fd = open(fifo_file, O_WRONLY , 0);
        strcpy(w_buf, line);
        if ((nwrite = write(fd, w_buf, 100)) == -1) {
            if (errno == EAGAIN)
                printf("The FIFO has not been read yet.Please try later\n");
        } else {
            printf("[write] %s\n",  w_buf);
        }
  //  }


}

int fiforead(int fifofile = 0) {
    char buf_r[100];
    int fd;
    int nread;
    char fifo_file[100];
    
    if(fifofile == 0){
        sprintf(fifo_file,"%s",FIFO);
    }else{
        sprintf(fifo_file,"%s.%d",FIFO,fifofile);
    }
    //printf("PIPE_BUF:%d\n",FIFO_BUF);

    debug("fiforead-block read start");
    if (mkfifo(fifo_file, O_CREAT | O_EXCL)) {
        debug("fiforead-fifo created before");
    } else {
        debug("fiforead-fifo create ok");
    }
    //printf("Preparing for reading bytes...\n");
    memset(buf_r, 0, sizeof (buf_r));
    fd = open(fifo_file, O_RDONLY, 0);
    if (fd == -1) {
        printf("fiforead-fifo open fail:%d\n", fd);

    } else {
        debug("fiforead-fifo open ok");
    }
    //while (1) {
        memset(buf_r, 0, sizeof (buf_r));
        if ((nread = read(fd, buf_r, 100)) == -1) {
            if (errno == EAGAIN) {
                //printf("no data yet\n");
            } else {
                //printf("read error:%d\n",errno);
            }
            //continue;
        }
        printf("fiforead.buf:%s\n",buf_r);
        int pid = atoi(buf_r);
        return pid;
}

int child_process() {
    int pid = fork();
    if (pid == 0) {
	return 0;
    }

    char buf_r[100];
    int fd;
    int nread;
    
    //printf("PIPE_BUF:%d\n",FIFO_BUF);

    debug("child_process.block read start");
    if (mkfifo(FIFO, O_CREAT | O_EXCL)) {
        debug("child_process.fifo created before");
    } else {
        debug("child_process.fifo create ok");
    }
    //printf("Preparing for reading bytes...\n");
    memset(buf_r, 0, sizeof (buf_r));
    fd = open(FIFO, O_RDONLY ,0);
    if (fd == -1) {
        printf("child_process.fifo open fail:%d\n", fd);

    } else {
        debug("child_process.fifo open ok");
    }
    while (1) {    
        memset(buf_r, 0, sizeof (buf_r));
        if ((nread = read(fd, buf_r, 100)) == -1) {
            if (errno == EAGAIN){
                //printf("no data yet\n");
            }else{
                //printf("read error:%d\n",errno);
            }
            //continue;
        }
        int pid = atoi(buf_r);
        printf("child_process.buf:%s-%d\n",buf_r,pid);
        //sleep(5);
        int shm_key = SHM_START + pid;
        int shm_sql_key = SHM_SQL_START + pid;
        char sql_buf[1000];
        shm_reader(shm_sql_key,sql_buf);
        dbproxy::sqlparam sqlparam;
        string line(sql_buf);
        sqlparam.ParseFromString(line);
        
        string sql = sqlBind(sqlparam);
        
        string resultLine;
        dbproxy::dbresult dbresult = proxy_mysql_query(sql); 
        dbresult.SerializeToString(&resultLine);
        
        printf("child_process.sql.reader:%s\n",sql.c_str());
        
        shm_writer(shm_key,resultLine.c_str());
        fifowrite(1,pid);
        printf("child_process.read %s from FIFO\n", buf_r);
        //break;
    }
    //unlink(FIFO);
    debug("done");
}

int fork_process(int num) {
    int i, k;
    int pid = -1;
    for (i = 0; i < num; i++) {
        if (pid != 0) {
            pid = fork();
            if (pid == 0) {
                child_process();
                break;
            } else {
                waitpid(pid,NULL,0);
            }
        } else {

            //printf("This is parent process[%i]\n",pid);
        }
    }
    return pid;
}

int epollfork() {
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
    my_addr.sin_port = htons(999);
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
                        socklen_t addr_len = sizeof ( struct sockaddr_in);
                        accept_fd = accept(listen_fd,
                                (struct sockaddr *) &remote_addr, &addr_len);
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
                        int pid = getpid();
                        int shm_sql_key = SHM_SQL_START + pid;
                        shm_writer(shm_sql_key,in_buf);
                        char hello[] = "Hello! (通讯)Socket communication world!\n";
                        fifowrite(pid);
                        int pid2 = fiforead(pid);
                        int shm_key = SHM_START+pid2;
                        char line[10000];
                        shm_reader(shm_key,line);
//                        printf("line:%s\n",line);
                        //int pid2 = 0;
                        printf("3->Child:%d,ProcessID:%d,EPOLLIN,fd:%d,recv:%s -->%d\n", child_id, getpid(), events[i].data.fd, in_buf,pid2);
                        if (send(events[i].data.fd, line, strlen(line), 0) == -1) {
                            fprintf(stderr, "Write Error:%s -%d\n", strerror(errno),pid2);
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
    
    //child_process();
    for(int i=0;i<5;i++){
    	child_process();
    }
    
    epollfork();
}
