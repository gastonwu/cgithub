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

#include "dna_multi_hash.h"
#include "dna_sharemem.h"
#include "game.h"

#define MAXBUF 6000
#define MAXEPOLLSIZE 10000

typedef CMultiHashCache<CHashUserData, HASH_ROW, HASH_COL> HashUser;//在线用户
typedef CMultiHashCache<CHashGameData, HASH_ROW, HASH_COL> HashGameWrite;//game信息写队列
typedef CMultiHashCache<CHashGameData, HASH_ROW, HASH_COL> HashGame;//在线游戏
HashUser *hashUser;
HashGameWrite *hashGameWrite;
HashGame *hashGame;

void errorMsg(PkgInfo& outputPkg,const char* line);

int initHashUser() {
    struct stShmInfo shm_info;
    shm_info.key = 0x20101230;
    shm_info.size = sizeof (HashUser) ;
    shm_info.shm_id = -1;
    shm_info.shm_addr = NULL;
    shm_info.log[0] = 0;

    bool new_created;
    if (GetShm(&shm_info, &new_created) == NULL) {
        //fprintf(stderr, shm_info.log);
        //printf("ha\n");
        return -1;
    }
    hashUser = (HashUser *) shm_info.shm_addr;

    return 0;
}
int initHashGameWrite() {
    struct stShmInfo shm_info;
    shm_info.key = 0x30101230;
    shm_info.size = sizeof (HashGame);
    shm_info.shm_id = -1;
    shm_info.shm_addr = NULL;
    shm_info.log[0] = 0;

    bool new_created;
    if (GetShm(&shm_info, &new_created) == NULL) {
        //fprintf(stderr, shm_info.log);
        //printf("ha\n");
        return -1;
    }
    hashGameWrite = (HashGame *) shm_info.shm_addr;

    return 0;
}
int initHashGame() {
    struct stShmInfo shm_info;
    shm_info.key = 0x40101230;
    shm_info.size = sizeof (HashGame);
    shm_info.shm_id = -1;
    shm_info.shm_addr = NULL;
    shm_info.log[0] = 0;

    bool new_created;
    if (GetShm(&shm_info, &new_created) == NULL) {
        //fprintf(stderr, shm_info.log);
        //printf("ha\n");
        return -1;
    }
    hashGame = (HashGame *) shm_info.shm_addr;

    return 0;
}
/*
   setnonblocking - 设置句柄为非阻塞方式
 */
int setnonblocking(int sockfd) {
    int v;
    v = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &v, sizeof (v));

    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK) == -1) {
        return -1;
    }
    return 0;
}

/**
 * 输入昵称登录
 * @param inputPkg
 * @param outputPkg
 */
void cmdLogin(PkgInfo& inputPkg, PkgInfo& outputPkg) {
    CHashUserData userData;
    memcpy(&userData, inputPkg.data, sizeof (userData));
    userData.gameMessageStatus = 0;
    userData.status = 0;
    memset(&userData.gameMessage,0,sizeof(userData.gameMessage));
    hashUser->InsertObj(userData);
    
    char msg[50];
    sprintf(msg, "%ld: login succc\n", userData.id);
    errorMsg(outputPkg, msg);
    printf("[ok]cmdLogin\n");
}

/**
 * 输入昵称登录
 * @param inputPkg
 * @param outputPkg
 */
void cmdLogout(PkgInfo& inputPkg, PkgInfo& outputPkg) {
    CHashUserData userData;
    //PkgInfo pkg;
    //memcpy( &pkg, buf, sizeof(pkg) );
    memcpy(&userData, inputPkg.data, sizeof (userData));
    //hashUser->InsertObj(userData);
    hashUser->RemoveObjByKey(userData.id);
    //htUser->InsertObj(userData);
    //printf("::%s\n", userData.nickName);
    printf("[ok]cmdLogout\n");
}


/**
 * 比赛出拳
 * @param inputPkg
 * @param outputPkg
 */
void cmdGamePlayHand(PkgInfo& inputPkg, PkgInfo& outputPkg) {
    CHashGameData gameData;
    //PkgInfo pkg;
    //memcpy( &pkg, buf, sizeof(pkg) );
    memcpy(&gameData, inputPkg.data, sizeof (gameData));
    //
    
    CHashGameData *gameDataMem = hashGame->GetObjByKey(gameData.id);
    printf("creater:%ld|%ld|%d,joiner:%ld|%ld|%d\n",gameDataMem->playA,gameDataMem->playACode,gameDataMem->playAScore,gameDataMem->playB,gameDataMem->playBCode,gameDataMem->playBScore);
    //printf("mem.id:%ld\n",gameDataMem->id);
    if(gameDataMem == NULL){
        printf("cmdGamePlayHand:game can not find\n");
        return;
    }
    //设定数据
    if(gameData.playACode !=0){
        if(gameDataMem->playA != gameData.playA){
            printf("cmdGamePlayHand:(A)you are not play in this game\n");
            return;
        }
        if(gameDataMem->playACode!=0){
            printf("cmdGamePlayHand:(A)you already play hand\n");
            printf("->creater:%ld|%ld|%d,joiner:%ld|%ld|%d\n",gameDataMem->playA,gameDataMem->playACode,gameDataMem->playAScore,gameDataMem->playB,gameDataMem->playBCode,gameDataMem->playBScore);
            return;
        }
        gameDataMem->playACode = gameData.playACode;
        
    }
    if(gameData.playBCode !=0){
        if(gameDataMem->playB != gameData.playB){
            printf("cmdGamePlayHand:(B)you are not play in this game\n");
            return;
        }
        if(gameDataMem->playBCode!=0){
            printf("cmdGamePlayHand:(B)you already play hand\n");
            return;
        }
        gameDataMem->playBCode = gameData.playBCode;
        
    }
    if( (gameDataMem->playACode == 0) || (gameDataMem->playBCode == 0)){
        hashGameWrite->InsertObj(*gameDataMem);
        printf("[game.id:%ld]->creater:%ld|%ld|%d,joiner:%ld|%ld|%d\n",gameData.id,gameDataMem->playA,gameDataMem->playACode,gameDataMem->playAScore,gameDataMem->playB,gameDataMem->playBCode,gameDataMem->playBScore);
         printf("cmdGamePlayHand:wait another player show hand\n");
         return;
    }
    //如果双方已出拳，比较结果
    //-平局
    int winner = 0;//0:平局,1:A,2:B
    if(gameDataMem->playACode == gameDataMem->playBCode){
        printf("cmdGamePlayHand: same code\n");
        winner = 0;
    }else{
        //3石头>2剪子>1(4)包袱
        //A-B == 1 || A=1(4) - B == 1
        winner = 2;
        if( (gameDataMem->playACode - gameDataMem->playBCode) == 1){
            winner = 1;
        }else if(gameDataMem->playACode == 1){
            if( (4 - gameDataMem->playBCode) == 1){
                winner = 1;
            }
        }
        
    }
    
    //加分-减分
    char msg[50];
    if(winner == 1){
        sprintf(msg,"%ld win\n",gameDataMem->playA);
        gameDataMem->playAScore = 3;
        gameDataMem->playBScore = -3;
    }else if(winner ==2 ){
        sprintf(msg,"%ld win\n",gameDataMem->playB);
        gameDataMem->playAScore = -3;
        gameDataMem->playBScore = 3;
    }else{
        gameDataMem->playAScore = 1;
        gameDataMem->playBScore = 1;
        sprintf(msg,"A=B\n");
    }
    
    //更新user状态
    CHashUserData *userAData = hashUser->GetObjByKey(gameDataMem->playA);
    CHashUserData *userBData = hashUser->GetObjByKey(gameDataMem->playB);
    memset(&userAData->gameMessage,0,sizeof(userAData->gameMessage));
    memset(&userBData->gameMessage,0,sizeof(userBData->gameMessage));
    
    memcpy(&userAData->gameMessage,msg,sizeof(userAData->gameMessage));
    memcpy(&userBData->gameMessage,msg,sizeof(userBData->gameMessage));
    userAData->gameMessageStatus = 1;
    userBData->gameMessageStatus = 1;
    hashUser->InsertObj(*userAData);
    hashUser->InsertObj(*userBData);
    
    //加入写hash
    printf(" write.inert game:::::::::::::::::::::\n");
    hashGameWrite->InsertObj(*gameDataMem);
    
    //复位game
    gameDataMem->playACode = 0;
    gameDataMem->playBCode = 0;
    hashGame->RemoveObjByKey(gameDataMem->id);
    //hashGame->InsertObj(*gameDataMem);
    printf("[ok]cmdGamePlayHand\n");

}

/**
 * 创建赛局
 * @param inputPkg
 * @param outputPkg
 */

void cmdGameCreate(PkgInfo& inputPkg, PkgInfo& outputPkg) {
    CHashGameData inputGameData;
    memcpy(&inputGameData, inputPkg.data, sizeof (inputGameData));
    //
    CHashGameData gameData;
    gameData.id = inputGameData.id;
    gameData.playA = inputGameData.id;
    gameData.playACode = 0;
    gameData.playAScore = 0;
    gameData.playB = 0;
    gameData.playBCode = 0;
    gameData.playBScore = 0;
    gameData.result = 0;
    
    printf("gameData.id:%ld\n",gameData.id);
    CHashGameData *gameDataMem = hashGame->GetObjByKey(gameData.id);
    char msg[50];
    if(gameDataMem == NULL){
        hashGame->InsertObj(gameData);
        sprintf(msg, "%ld: game create success\n", gameData.id);        
    }else{
        sprintf(msg, "%ld: game already created\n", gameData.id);        
    }
    errorMsg(outputPkg, msg);
    printf("[ok]cmdGameCreate\n");
}


void errorMsg(PkgInfo& outputPkg,const char* line){
    outputPkg.id = 14;
    struct MsgInfo msg;
    memset(&msg, 0, sizeof (msg));
    memcpy(msg.message,line,sizeof(msg.message));
    memcpy(outputPkg.data, (char *) &msg, sizeof (outputPkg.data));
}
/**
 * 查询赛局
 * @param inputPkg
 * @param outputPkg
 */
void cmdGameQuery(PkgInfo& inputPkg, PkgInfo& outputPkg) {
    CHashGameData gameData;
    memcpy(&gameData, inputPkg.data, sizeof (gameData));
    //
    
    CHashGameData *gameDataMem = hashGame->GetObjByKey(gameData.id);
    //gameData.id = gameDataMem->id;
    gameData.playA = gameDataMem->playA;
    gameData.playACode = gameDataMem->playACode;
    gameData.playB = gameDataMem->playB;
    gameData.playBCode = gameDataMem->playBCode;
    printf("creater:%ld|%ld|%d,joiner:%ld|%ld|%d\n",gameDataMem->playA,gameDataMem->playACode,gameDataMem->playAScore,gameDataMem->playB,gameDataMem->playBCode,gameDataMem->playBScore);
     
    //printf("mem.id:%ld\n",gameDataMem->id);
    if(gameDataMem == NULL){
        char msg[50];
        sprintf(msg,"can not find game:%ld\n",gameData.id);
        errorMsg(outputPkg,msg);
    }else{
        outputPkg.id = inputPkg.id;
        printf(" ->%ld: join in the game:%ld.\n",gameData.playB,gameData.playA);
                    
        memcpy(&outputPkg.data, (char *) &gameData, sizeof (outputPkg.data));
    }
    printf("[ok]cmdGameQuery\n");
}
/** 135 7630 9308 - 0793 8035336 
 * 查询用户
 * @param inputPkg
 * @param outputPkg
 */
void cmdUserQuery(PkgInfo& inputPkg, PkgInfo& outputPkg) {
    CHashUserData userData;
    memcpy(&userData, inputPkg.data, sizeof (userData));
    //
    printf("pkg.id:%d-%ld\n",inputPkg.id,userData.id);
    
    CHashUserData *userDataMem = hashUser->GetObjByKey(userData.id);
    if(userDataMem->id) {
        printf("user:%ld|%d|%s\n", userDataMem->id, userDataMem->gameMessageStatus, userDataMem->gameMessage);
    }
//    
    if(userDataMem == NULL){
        char msg[50];
        sprintf(msg,"can not find user:%ld\n",userData.id);
        printf("->1\n");
        errorMsg(outputPkg,msg);
    }else{
        printf("->2\n");
        outputPkg.id = inputPkg.id;
        printf("gameMessageStatus:%d\n",userDataMem->gameMessageStatus);
        //printf(" ->%ld: join in the game:%ld.\n",gameData.playB,gameData.playA);
                    
        memcpy(&outputPkg.data, (char *) userDataMem, sizeof (outputPkg.data));
    }
    printf("[ok]cmdUserQuery\n");
}




/**
 * 加入赛局
 * @param inputPkg
 * @param outputPkg
 */
void cmdGameJoin(PkgInfo& inputPkg, PkgInfo& outputPkg) {
    CHashGameData gameData;
    //PkgInfo pkg;
    //memcpy( &pkg, buf, sizeof(pkg) );
    memcpy(&gameData, inputPkg.data, sizeof (gameData));
    //
    printf(" join game id:%ld\n",gameData.id);
    CHashGameData *gameDataMem = hashGame->GetObjByKey(gameData.id);
    //printf("mem.id:%ld\n",gameDataMem->id);
    if(gameDataMem == NULL){
        char msg[50];
        sprintf(msg,"can not find game:%ld\n",gameData.id);
        errorMsg(outputPkg,msg);
    }else if(gameDataMem->playB == 0){
        gameDataMem->playB = gameData.playB;
        hashGame->InsertObj(*gameDataMem);
        char msg[50];
        sprintf(msg,"game %ld join success\n",gameData.id);
        errorMsg(outputPkg,msg);
        outputPkg.id = 3;
    }else{
        char msg[50];
        sprintf(msg,"game %ld already full\n",gameData.id);
        errorMsg(outputPkg,msg);
    }
    printf("[ok]cmdGameJoin\n");
}


/**
 * 查询在线賽局列表
 * @param inputPkg
 * @param outputPkg
 */
void cmdQueryGameList(PkgInfo& inputPkg, PkgInfo& outputPkg) {
    struct GameList list;
    memset(&list, 0, sizeof (list));
    //memset(&list,0,sizeof(list));
    int pos = 0;
    for (int r = 0; r < HASH_ROW; ++r) {
        for (int c = 0; c < HASH_COL; ++c) {
            CHashGameData *p = &(hashGame->m_items[r][c]);
            if (p->id) {
                list.game[pos] = *p;
                //list.key[pos] = pos+1;
                pos++;
                printf("->%lu\n", p->id);
                //++count;
                //printf("%s\n", get_ip_str_n(ht->m_items[r][c].m_val));
            }
        }
    }
    outputPkg.id = inputPkg.id;
    memcpy(outputPkg.data, (char *) &list, sizeof (outputPkg.data));

    for (int i = 0; i < LIST_GAME_LEN; i++) {
        if (list.game[i].id) {
            printf("cmdQueryGameList->%ld\n", list.game[i].id);
        }
    }
    printf("[ok]cmdQueryGameList\n");
}



void cmdQueryUserList(PkgInfo& inputPkg, PkgInfo& outputPkg) {
    struct UserList list;
    memset(&list, 0, sizeof (list));
    //memset(&list,0,sizeof(list));
    int pos = 0;
    for (int r = 0; r < HASH_ROW; ++r) {
        for (int c = 0; c < HASH_COL; ++c) {
            CHashUserData *p = &(hashUser->m_items[r][c]);
            if (p->id) {
                list.user[pos] = *p;
                //list.key[pos] = pos+1;
                pos++;
                //printf("->%lu\n", p->id);
                //++count;
                //printf("%s\n", get_ip_str_n(ht->m_items[r][c].m_val));
            }
        }
    }
    outputPkg.id = inputPkg.id;
    memcpy(outputPkg.data, (char *) &list, sizeof (outputPkg.data));

    for (int i = 0; i < LIST_USER_LEN; i++) {
        if (list.user[i].id) {
            printf("cmdQueryUserList->%ld\n", list.user[i].id);
        }
    }
    printf("[ok:]cmdQueryUserList\n");
}

void cmdTest(PkgInfo& inputPkg, PkgInfo& outputPkg) {
    CHashGameData gameData;
    memcpy(&gameData, inputPkg.data, sizeof (gameData));
    //
    
    //CHashGameData *gameDataMem = hashGame->GetObjByKey(gameData.id);
    if (0) {
        CHashGameData *gameDataMem;
        gameDataMem->id = 11111;
        gameDataMem->playA = 11111;
        gameDataMem->playACode = 1;
        gameDataMem->playB = 11112;
        gameDataMem->playBCode = 2;

        outputPkg.id = inputPkg.id;
        memcpy(&outputPkg.data, (char *) (CHashGameData*)gameDataMem, sizeof (outputPkg.data));
    } else {

        CHashGameData gameDataMem;
        gameDataMem.id = 11111;
        gameDataMem.playA = 11111;
        gameDataMem.playACode = 1;
        gameDataMem.playB = 11112;
        gameDataMem.playBCode = 2;

        outputPkg.id = inputPkg.id;
        memcpy(&outputPkg.data, (char *) &gameDataMem, sizeof (outputPkg.data));
    }
    
    printf("[ok]cmdTest\n");
}

/*
   handle_message - 处理每个 socket 上的消息收发
 */
int handle_message(int new_fd) {
    char buf[MAXBUF + 1];
    int len;
    
//    while(1){
        /* 开始处理每个新连接上的数据收发 */
        bzero(buf, MAXBUF + 1);
        /* 接收客户端的消息 */
        len = recv(new_fd, buf, MAXBUF, 0);
        buf[len] = 0;
        if (len > 0) {
            printf("%d接收消息成功，共%d个字节的数据\n", new_fd, len);

            CHashUserData userData;
            PkgInfo inputPkg;
            PkgInfo outputPkg;
            memcpy(&inputPkg, buf, sizeof (inputPkg));
            
            printf("-------%d----------\n",new_fd);
            //cmdTest
            if (inputPkg.id == 1) {
                cmdLogin(inputPkg, outputPkg);
            } else if(inputPkg.id == 6){
                cmdUserQuery(inputPkg,outputPkg);
            } else if (inputPkg.id == 15) {
                cmdTest(inputPkg, outputPkg);
            } else if (inputPkg.id == 13) {
                cmdLogout(inputPkg, outputPkg);
            } else if (inputPkg.id == 5) {
                cmdGameQuery(inputPkg, outputPkg);
            } else if (inputPkg.id == 12) {
                cmdQueryUserList(inputPkg, outputPkg);
            } else if(inputPkg.id == 2){
                cmdGameCreate(inputPkg, outputPkg);
            }else if(inputPkg.id == 11){
                cmdQueryGameList(inputPkg,outputPkg);
            }else if(inputPkg.id == 3){
                cmdGameJoin(inputPkg,outputPkg);
            }else if(inputPkg.id == 4){
                cmdGamePlayHand(inputPkg,outputPkg);
            }else {
                printf("cmd unknow\n");
            }

            if (send(new_fd, (char *) &outputPkg, sizeof (outputPkg), 0) == -1) {
                fprintf(stderr, "Write Error:%s\n", strerror(errno));
                //exit(1);
            }

        } else {
            if (len < 0){
                printf("消息接收失败！错误代码是%d，错误信息是'%s'，len:%d\n",errno, strerror(errno),len);
            }
        }
//    }
    //close(new_fd);
    /* 处理每个新连接上的数据收发结束 */
    return len;
}


#define EPOLL_SIZE 100
#define EVENT_ARR 200
#define BACK_QUEUE 10
#define PORT 9995
#define BUF_SIZE 16

//void setnonblocking(int sockFd) {
//    int opt;
//
//    //获取sock原来的flag
//    opt = fcntl(sockFd, F_GETFL);
//    if (opt < 0) {
//        printf("fcntl(F_GETFL) fail.");
//        exit(-1);
//    }
//
//    //设置新的flag,非阻塞
//    opt |= O_NONBLOCK;
//    if (fcntl(sockFd, F_SETFL, opt) < 0) {
//        printf("fcntl(F_SETFL) fail.");
//        exit(-1);
//    }
//}


int serverStart() {
    int pid = fork();
    if(pid ==0 ){
        return 0;
    }

    int serverFd;

    //创建服务器fd
    serverFd = socket(PF_INET, SOCK_STREAM, 0);
    setnonblocking(serverFd);

    //创建epoll，并把serverFd放入监听队列
    int epFd = epoll_create(EPOLL_SIZE);
    struct epoll_event ev, evs[EVENT_ARR];
    ev.data.fd = serverFd;
    ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epFd, EPOLL_CTL_ADD, serverFd, &ev);

    //绑定服务器端口
    struct sockaddr_in serverAddr;
    socklen_t serverLen = sizeof (struct sockaddr);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_family = AF_INET;
    //    if (bind(serverFd, (struct sockaddr *) &serverAddr, sizeof (struct sockaddr))== -1) {
    //        perror("bind");
    //        exit(1);
    //    }
    if (bind(serverFd, (struct sockaddr *) &serverAddr, sizeof (struct sockaddr)) == -1) {
        printf("bind() fail:%s.\n", strerror(errno));
        exit(-1);
    } else {
        printf("bind() success.\n");
    }

    //打开监听
    if (listen(serverFd, BACK_QUEUE)) {
        printf("Listen fail.\n");
        exit(-1);
    } else {
        printf("listen() success.\n");
    }

    //死循环处理
    int clientFd;
    struct sockaddr_in clientAddr;
    socklen_t clientLen;
    char buf[BUF_SIZE];

    while (1) {
        //等待epoll事件的到来，最多取EVENT_ARR个事件
        int nfds = epoll_wait(epFd, evs, EVENT_ARR, -1);
        //处理事件
        for (int i = 0; i < nfds; i++) {
            if (evs[i].data.fd == serverFd && evs[i].data.fd & EPOLLIN) {
                //如果是serverFd,表明有新连接连入
                if ((clientFd = accept(serverFd,
                        (struct sockaddr *) &clientAddr, &clientLen)) < 0) {
                    printf("accept fail.\n");
                }
                printf("Connect from %s:%d\n", inet_ntoa(clientAddr.sin_addr),htons(clientAddr.sin_port));
                setnonblocking(clientFd);
                //注册accept()到的连接
                ev.data.fd = clientFd;
                ev.events = EPOLLIN | EPOLLET;
                epoll_ctl(epFd, EPOLL_CTL_ADD, clientFd, &ev);
            } else if (evs[i].events & EPOLLIN) {
                //如果不是serverFd,则是client的可读
                if ((clientFd = evs[i].data.fd) > 0) {
                    int len = handle_message(clientFd);
                    //先进行试探性读取
                    //int len = read(clientFd, buf, BUF_SIZE);
                    if (len > 0) {
                        continue;
                    } else if (len == 0) {
                        //出发了EPOLLIN事件，却没有可以读取的，表示断线
                        printf("Client closed at %d\n", clientFd);
                        epoll_ctl(epFd, EPOLL_CTL_DEL, clientFd, &ev);
                        close(clientFd);
                        evs[i].data.fd = -1;
                        break;
                    } else if (len == EAGAIN) {
                        continue;
                    } else {
                        //client读取出错
                        printf("read() fail.");
                    }
                }
            } else {
                printf("other event.\n");
            }
        }
    }

    return 0;
}
/**
 * 异步更新比赛信息到文件
 */
void cmdLoopGameWriteList() {
    int pid = fork();
    if(pid ==0 ){
        return;
    }
    FILE *fp;
    fp = fopen("game.data", "ab+");
    printf("cmdLoopGameWriteList start\n");
    while (1) {
        int pos = 0;
        sleep(1);
        for (int r = 0; r < HASH_ROW; ++r) {
            for (int c = 0; c < HASH_COL; ++c) {
                CHashGameData *p = &(hashGameWrite->m_items[r][c]);
                if (p->id) {
                    char buffer[150];
                    memset(buffer,0,sizeof(buffer));
                    sprintf(buffer,"%ld|%ld|%d|%ld|%ld|%d\n",p->playA,p->playACode,p->playAScore,p->playB,p->playBCode,p->playBScore);
                    
                    fwrite (buffer , 1 , sizeof(buffer) , fp );
                    //list.game[pos] = *p;
                    //list.key[pos] = pos+1;
                    pos++;
                    printf("loop.write->%lu\n", p->id);
                    //++count;
                    //printf("%s\n", get_ip_str_n(ht->m_items[r][c].m_val));
                    hashGameWrite->RemoveObjByKey(p->id);
                }
            }
        }
        fflush(fp);
    }
    fclose(fp);

}


int main(){
    initHashUser();
    initHashGameWrite();
    initHashGame();
    
    cmdLoopGameWriteList();
    serverStart();
}