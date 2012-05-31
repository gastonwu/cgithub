#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "game.h"

#include "dna_multi_hash.h"
#include "dna_sharemem.h"

#define MAXBUF 6000

void loopWaitJoin(int fd);
int cmdGamePlayHandA(int fd);
int cmdGamePlayHandB(int fd);
int cmdGameQuery(int fd,int gameId,CHashGameData &gameData);
int cmdQueryGameList(int fd);
int cmdUserQuery(int fd,int userId,CHashUserData &userData);

int loginId=0;
char loginNick[50];
int currentGameId=0;
int joinGameId=0;
/**
 * 昵称登陆
 * @param fd
 * @return 
 */
int cmdLogin(int fd) {
    PkgInfo inputPkg;
    PkgInfo outputPkg;
    
    if(loginId !=0){
        printf(" %d: you already login\n",loginId);
        return 0;
    }
    
    char loginIdInput[50];
    printf("  input your id:");
    scanf("%d", &loginId);
    printf("  input your nick:");
    scanf("%s", loginNick);
    

    CHashUserData user;
    user.id = loginId;
    memcpy(user.nickName, loginNick, sizeof (loginNick));
    //user.score = 99;

    outputPkg.id = 1;//cmd登陆
    memcpy(&outputPkg.data, (char *) &user, sizeof (user));
    send(fd, (char *) &outputPkg, sizeof (outputPkg), 0);
    
    
    //recv
    char buf[MAXBUF + 1];
    recv(fd, buf, MAXBUF, 0);
    memcpy(&inputPkg, buf, sizeof (inputPkg));
    if(inputPkg.id == 14){
        struct MsgInfo msg;
        memset(&msg,0,sizeof(msg));
        memcpy(&msg, inputPkg.data, sizeof(msg));
        printf(" [server]:%s\n",msg.message);
    }
    
}
/**
 * 退出登陆
 * @param fd
 * @return 
 */
int cmdLogout(int fd) {
    PkgInfo inputPkg;
    PkgInfo outputPkg;

    if (loginId != 0) {
        CHashUserData user;
        user.id = loginId;
        //memcpy(user.nickName, "gaston3", sizeof ("gaston3"));
        //user.score = 99;

        outputPkg.id = 13; //cmd登陆
        memcpy(&outputPkg.data, (char *) &user, sizeof (outputPkg.data));

        send(fd, (char *) &outputPkg, sizeof (outputPkg), 0);
    }
    close(fd);
}
/**
 * 创建赛局
 * @param fd
 * @return 
 */
int cmdGameCreate(int fd) {
    PkgInfo inputPkg;
    PkgInfo outputPkg;
    
    if(loginId == 0){
        cmdLogin(fd);
    }
    
    CHashGameData gameData;
    gameData.id = loginId;
    gameData.playA = loginId;

    outputPkg.id = 2;//cmd创建战局
    memcpy(&outputPkg.data, (char *) &gameData, sizeof (gameData));

    send(fd, (char *) &outputPkg, sizeof (outputPkg), 0);
    //recv
    char buf[MAXBUF + 1];
    recv(fd, buf, MAXBUF, 0);
    memcpy(&inputPkg, buf, sizeof (inputPkg));
    if(inputPkg.id == 14){
        struct MsgInfo msg;
        memset(&msg,0,sizeof(msg));
        memcpy(&msg, inputPkg.data, sizeof(msg));
        printf(" [server]:%s\n",msg.message);
    }
    
    currentGameId=loginId;
    loopWaitJoin(fd);
    cmdGamePlayHandA(fd);
}


void loopWaitJoin(int fd){
    PkgInfo inputPkg;
    PkgInfo outputPkg;
    CHashGameData gameData;
    int ret = 0;
    while(1){
        sleep(1);
        memset(&gameData,0,sizeof(gameData));
        ret = cmdGameQuery(fd,loginId,gameData);
        if(ret == 0){
            
            //printf(" %ld: join in the game.\n",gameData.playB);
            printf(" %ld: join in the game:%ld.\n",gameData.playB,gameData.playA);
            break;
        }else{
            printf("loop-->%d\n",fd);
        }
        
    }
}

void loopWaitGameResult(int fd){
    PkgInfo inputPkg;
    PkgInfo outputPkg;
    CHashUserData userData;
    int ret = 0;
    while(1){
        sleep(1);
        memset(&userData,0,sizeof(userData));
        ret = cmdUserQuery(fd,loginId,userData);
        if(ret == 0){
            printf(" [server]%s\n",userData.gameMessage);
            //printf(" %ld: join in the game:%ld.\n",gameData.playB,gameData.playA);
            break;
        }else{
            printf("loop-->%d\n",fd);
        }
        
    }
    
}

/**
 * 查询用户信息
 * @param fd
 * @return 
 */
int cmdUserQuery(int fd,int userId,CHashUserData &userData) {
    PkgInfo inputPkg;
    PkgInfo outputPkg;
    
    CHashUserData user;
    user.id = userId;
    //printf("::query.game.id:%d\n",gameId);
    //game.playB=loginId;

    outputPkg.id = 6;//cmd查询用户信息
    memcpy(&outputPkg.data, (char *) &user, sizeof (outputPkg.data));
    send(fd, (char *) &outputPkg, sizeof (outputPkg), 0);
    
    
    //recv
    char buf[MAXBUF + 1];
    int len = recv(fd, buf, MAXBUF, 0);
    memcpy(&inputPkg, buf, sizeof (inputPkg));
    printf(" len:%d struct.len:%d\n",len,sizeof(inputPkg));
    if(inputPkg.id == 14){
        struct MsgInfo msg;
        memset(&msg,0,sizeof(msg));
        memcpy(&msg, inputPkg.data, sizeof(msg));
        printf(" [server]:%s\n",msg.message);
        return -1;
    }else if(inputPkg.id == 6){
        //memset(&gameData,0,sizeof(gameData));
        printf("-1:%d -2:%d\n",sizeof(CHashUserData),sizeof(inputPkg.data));
        memcpy(&userData, inputPkg.data, sizeof (userData));
        printf(" ->user.id:%ld\n\n",userData.id);
        if(userData.gameMessageStatus == 0){
            return -2;
        }else{
            //printf(" ->%ld: join in the game:%ld.\n",gameData.playB,gameData.playA);
            return 0;
        }
        
        
    }
    
}


/**
 * 查询赛局
 * @param fd
 * @return 
 */
int cmdGameQuery(int fd,int gameId,CHashGameData &gameData) {
    PkgInfo inputPkg;
    PkgInfo outputPkg;
    
    CHashGameData game;
    game.id = gameId;
    printf("::query.game.id:%d\n",gameId);
    //game.playB=loginId;

    outputPkg.id = 5;//cmd查询战局
    memcpy(&outputPkg.data, (char *) &game, sizeof (outputPkg.data));
    send(fd, (char *) &outputPkg, sizeof (outputPkg), 0);
    
    
    //recv
    char buf[MAXBUF + 1];
    int len = recv(fd, buf, MAXBUF, 0);
    memcpy(&inputPkg, buf, sizeof (inputPkg));
    printf(" len:%d struct.len:%d\n",len,sizeof(inputPkg));
    if(inputPkg.id == 14){
        struct MsgInfo msg;
        memset(&msg,0,sizeof(msg));
        memcpy(&msg, inputPkg.data, sizeof(msg));
        printf(" [server]:%s\n",msg.message);
        return -1;
    }else if(inputPkg.id == 5){
        //memset(&gameData,0,sizeof(gameData));
        printf("-1:%d -2:%d\n",sizeof(CHashGameData),sizeof(inputPkg.data));
        memcpy(&gameData, inputPkg.data, sizeof (gameData));
        printf(" ->gameData.id:%ld\n ->gameData.playA:%ld\n",gameData.id,gameData.playA);
        if(gameData.playB == 0){
            return -2;
        }else{
            printf(" ->%ld: join in the game:%ld.\n",gameData.playB,gameData.playA);
            return 0;
        }
        
        
    }
    
}

/**
 * 加和赛局
 * @param fd
 * @return 
 */
int cmdGameJoin(int fd) {
    PkgInfo inputPkg;
    PkgInfo outputPkg;
    
    if(loginId == 0){
        cmdLogin(fd);
    }

    cmdQueryGameList(fd);//查询在线游戏列表
    printf("  please input join gameid :");
    scanf("%d", &joinGameId);//输入加入游戏id

    CHashGameData game,gameData;
    game.id = joinGameId;
    game.playB=loginId;
    
//    int ret = cmdGameQuery(fd,joinGameId,gameData);
//    if(ret< 0){
//        printf(" can not find game:%d\n",joinGameId);
//        cmdGameJoin(fd);
//        return 0;
//    }

    outputPkg.id = 3;//cmd加入战局
    memcpy(&outputPkg.data, (char *) &game, sizeof (game));

    send(fd, (char *) &outputPkg, sizeof (outputPkg), 0);
    
    //recv
    char buf[MAXBUF + 1];
    recv(fd, buf, MAXBUF, 0);
    memcpy(&inputPkg, buf, sizeof (inputPkg));
    if(inputPkg.id == 14){
        struct MsgInfo msg;
        memset(&msg,0,sizeof(msg));
        memcpy(&msg, inputPkg.data, sizeof(msg));
        printf(" [server]:%s\n",msg.message);
        cmdGameJoin(fd);
    }else if(inputPkg.id == 3){
        struct MsgInfo msg;
        memset(&msg,0,sizeof(msg));
        memcpy(&msg, inputPkg.data, sizeof(msg));
        printf(" [server]:%s\n",msg.message);        
    }
    cmdGamePlayHandB(fd);
}

/**
 * 出拳A
 * @param fd
 * @return 
 */
int cmdGamePlayHandA(int fd) {
    PkgInfo inputPkg;
    PkgInfo outputPkg;

    int code = 0;
    printf("  please input your number(3石头>2剪子>1包袱) :");
    scanf("%d", &code);//输入加入游戏id
    
    if(code < 1 || code > 4){
        printf(" you input is: %d,please between[1,3]\n",code);
        return cmdGamePlayHandA(fd);
    }

    CHashGameData game;
    game.id = loginId;
    game.playA=loginId;
    game.playACode = code;
    //memcpy(user.nickName, "gaston3", sizeof ("gaston3"));
    //user.score = 99;

    outputPkg.id = 4;//cmd出拳
    memcpy(&outputPkg.data, (char *) &game, sizeof (game));

    send(fd, (char *) &outputPkg, sizeof (outputPkg), 0);
    //sleep(2);
    //检查比赛结果
    loopWaitGameResult(fd);
}

/**
 * 出拳B
 * @param fd
 * @return 
 */
int cmdGamePlayHandB(int fd) {
    PkgInfo inputPkg;
    PkgInfo outputPkg;

    int code = 0;
    printf("  please input your number(3石头>2剪子>1包袱) :");
    scanf("%d", &code);//输入加入游戏id
    
    if(code < 1 || code > 4){
        printf(" you input is: %d,please between[1,3]\n",code);
        return cmdGamePlayHandB(fd);
    }

    CHashGameData game;
    game.id = joinGameId;
    game.playB=loginId;
    game.playBCode = code;

    outputPkg.id = 4;//cmd出拳
    memcpy(&outputPkg.data, (char *) &game, sizeof (game));

    send(fd, (char *) &outputPkg, sizeof (outputPkg), 0);
    //sleep(2);
    //检查比赛结果
    loopWaitGameResult(fd);
    
}




/**
 * 查询用户在线列表
 * @param fd
 * @return 
 */
int cmdQueryUserList(int fd){
    PkgInfo inputPkg;
    PkgInfo outputPkg;

    outputPkg.id = 12;
    send(fd, (char *) &outputPkg, sizeof (outputPkg), 0);
    
    //recv
    char buf[MAXBUF + 1];
    int len;
    /* 开始处理每个新连接上的数据收发 */
    
    bzero(buf, MAXBUF + 1);
    /* 接收客户端的消息 */
    len = recv(fd, buf, MAXBUF, 0);
    memcpy(&inputPkg, buf, sizeof (inputPkg));
    //printf("recv-cmd_id:%d\n",cmdId);
    //printf("len:%d\n",len);
    struct UserList list;
    memset(&list,0,sizeof(list));
    
    memcpy(&list, inputPkg.data, sizeof(inputPkg.data));
    
    for(int i=0;i<LIST_USER_LEN;i++){
        if(list.user[i].id){
            printf("->%ld\n",list.user[i].id);
        }
    }
}

/**
 * 查询赛局在线列表
 * @param fd
 * @return 
 */
int cmdQueryGameList(int fd){
    PkgInfo inputPkg;
    PkgInfo outputPkg;

    outputPkg.id = 11;
    send(fd, (char *) &outputPkg, sizeof (outputPkg), 0);
    
    //recv
    char buf[MAXBUF + 1];
    int len;
    /* 开始处理每个新连接上的数据收发 */
    
    bzero(buf, MAXBUF + 1);
    /* 接收客户端的消息 */
    len = recv(fd, buf, MAXBUF, 0);
    memcpy(&inputPkg, buf, sizeof (inputPkg));
    printf("list.len:%d\n",sizeof(inputPkg));
    //printf("recv-cmd_id:%d\n",cmdId);
    //printf("len:%d\n",len);
    struct GameList list;
    memset(&list,0,sizeof(list));
    memcpy(&list, inputPkg.data, sizeof(inputPkg.data));
    
    for(int i=0;i<LIST_USER_LEN;i++){
        if(list.game[i].id){
            printf("->%ld\n",list.game[i].id);
        }
    }
}

/**
 * Test
 * @param fd
 * @return 
 */
int cmdTest(int fd){
    PkgInfo inputPkg;
    PkgInfo outputPkg;

    outputPkg.id = 6;
    struct CHashUserData userData;
    userData.id = 888;
    memcpy(&outputPkg.data,(char *)&userData,sizeof(userData));
    
    send(fd, (char *) &outputPkg, sizeof (outputPkg), 0);
    
    //recv
    char buf[MAXBUF + 1];
    int len;
    bzero(buf, MAXBUF + 1);

    len = recv(fd, buf, MAXBUF, 0);
    printf("len:%d\n",len);
    memcpy(&inputPkg, buf, sizeof (inputPkg));
    printf("recv.id:%d\n",inputPkg.id);
    struct CHashUserData userDataRecv;
//    memset(&userDataRecv,0,sizeof(userDataRecv));
    memcpy(&userDataRecv, inputPkg.data, sizeof(userDataRecv));
    printf("recv.id:%ld\n",userDataRecv.id);
    printf("game.status:%d\n",userDataRecv.gameMessageStatus);
    
}


void cmdShowMenu(){
    printf("1:登陆.提交昵称\n");
    printf("2:开设战局\n");
    printf("3:加入战局\n");
    printf("41:A出拳\n");
    printf("42:B出拳\n");
    printf("5:查询战局\n");

    printf("11:查询战局列表\n");
    printf("12:查询在线人线\n");
    printf("120:返回在线人\n");
    printf("13:退出登陆\n");    
}
int menuCmd(int sockfd) {
    char inputLine [80];
    int cmdId = 0;

    
    
    printf("please input number(show menu:0): ");
    
    scanf("%s", inputLine);
    cmdId = atoi(inputLine);
    if (cmdId == 0){
        cmdShowMenu();
    }if (cmdId == 1) {
        cmdLogin(sockfd);
    } else if (cmdId == 13) {
        cmdLogout(sockfd);
        return 1;
    } else if (cmdId == 12) {
        cmdQueryUserList(sockfd);
    } else if (cmdId == 2) {
        cmdGameCreate(sockfd);
    } else if (cmdId == 5) {
        //cmdGameQuery(sockfd);
    } else if (cmdId == 11) {
        cmdQueryGameList(sockfd);
    } else if (cmdId == 3) {
        cmdGameJoin(sockfd);
    } else if (cmdId == 41) {
        cmdGamePlayHandA(sockfd);
    } else if (cmdId == 42) {
        cmdGamePlayHandB(sockfd);
    } else {
        printf("please select number from menu,thanks\n");
    }
    return 0;

    //    cmdLogin(sockfd); //昵称登陆
    //    cmdLogout(sockfd); //退出登陆
    //    cmdQueryUserList(sockfd); //查询在线用户列表
    //    cmdGameCreate(sockfd); //创建战局
    //    cmdQueryGameList(sockfd); //查询战局
    //    cmdGameJoin(sockfd); //加入战局
    //    cmdGamePlayHandA(sockfd); //play a:出拳
    //    cmdGamePlayHandB(sockfd); //play b:出拳    
}

int main(int argc, char**argv) {



    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct hostent *server = gethostbyname("127.0.0.1");
    if (server == NULL) {
        printf("ERROR, no such host\n");
        exit(0);
    }

    struct sockaddr_in serv_addr;
    memset((char*) &serv_addr, 0, sizeof (serv_addr));

    serv_addr.sin_family = AF_INET;
    memcpy((char*) server->h_addr,
            (char*) &serv_addr.sin_addr.s_addr,
            server->h_length);
    serv_addr.sin_port = htons(9995);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0) {
        printf("error connect:%d\n", errno);
        exit(0);
    }

    //cmdTest(sockfd);
   
    int ret = 0;
    while(1){
       ret = menuCmd(sockfd); 
       if(ret == 1){
           break;
       }
    }
    //todo
    //1.改成长连接，选项从client输入 [done]
    //2.赛局结果写入文件
    //3.积分写入DB(可选做不做)
    //4.出错处理
    //5.返回处理 [done]
    //6.超时检查
    //7.加入战局处理 [doing]
    close(sockfd);

    return 0;
}