/* 
 * File:   main.cpp
 * Author: gaston
 *
 * Created on 2012年9月17日, 上午10:43
 */

#include <stdio.h>
#include <string.h>
#include "queue_shm.h"

struct Msg{
        char line[100];
}typedef Msg;

//typedef QueueShm<Msg, 10> NetDataQueueShm;
//NetDataQueueShm* netdataQueue;

int main(int argc, char** argv) {
//    QShm qshm;
//    qshm.init(12345,sizeof(NetDataQueueShm));
//    netdataQueue = (NetDataQueueShm *)qshm.getBuf();
    
    QueueShm<Msg, 10> netdataQueue;
//NetDataQueueShm* netdataQueue;
    
    Msg msg;
    memset(&msg,0,sizeof(msg));
    memcpy(&msg.line,"abc",sizeof("abc"));
    int loop=0;
    for(int i=0;i<10;i++){
        printf("[%d]\t",loop++);
        netdataQueue.push(msg);
        netdataQueue.printStatus();
    }
    printf("[%d]-\t",loop++);
    netdataQueue.pop(msg);
    netdataQueue.printStatus();
    for(int i=0;i<5;i++){
//        printf("(%c)\t",'-');netdataQueue.printStatus();
        printf("[%d]\t",loop++);
        netdataQueue.push(msg);
        netdataQueue.printStatus();
    }
    
    netdataQueue.printStatus();
    return 0;
}