/* 
 * File:   main.cpp
 * Author: gaston
 *
 * Created on 2012年9月12日, 下午5:25
 */

#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <iostream>
#include <string.h>

using namespace std; 

#define BUFF_RESERVE_LENGTH 8
struct QHead{
    int leftSize;//剩余空间
    int head;//读指针
    int tail;//写指针
    int offset;//步长
}typedef QHead;

class QShm{
public:
    ~QShm(){
        if(buf != NULL){
            //shmdt(buf);
        }
    }
    void init(int key,int size){
        int shmId = shmget(key,size,IPC_CREAT | 600);
        if(shmId < 0){
            printf("QShm.init:shmget error\n");
            return;
        }
        buf = (char *)shmat(shmId,NULL,0);
        if(buf == NULL){
            printf("QShm.init:shmat error\n");
            return;
        }
        QHead qHead;
        qHead.head = 0;
        qHead.tail = 0;
        qHead.offset = sizeof(QHead);
        qHead.leftSize = size - qHead.offset;
        memcpy(buf,(char *) &qHead,sizeof(qHead));
        
    }
    char* getBuf(){
        return buf;
    }
private:
    char* buf;
    size_t size;
    key_t key;
};
class QSQueue{
public:
    QSQueue(char *pbuf){
        qHead = (QHead *)pbuf;
        buf = pbuf+sizeof(QHead);
    }
    int getItem(char* item,int* size){
        int leftSize = 0;
        if(item == NULL) return -1;
        if(size == NULL) return -2;
        if(qHead->offset <= 0) return -3;
        if(qHead->leftSize <= 0) return -4;
        int head = qHead->head;
        int tail = qHead->tail;
        
        
    }
    int putItem(const char* item,int* size){
        if(item == NULL) return -1;
        if(size == NULL) return -2;
        if(qHead->offset <= 0) return -3;
        if(qHead->leftSize <= 0) return -4;
        //取头尾
        int head = qHead->head;
        int tail = qHead->tail;
        //满
        if(isFull()){
            return -5;
        }
        //剩余缓冲大小，小于包长度字节数，错误返回
        int leftSize = getLeftSize();

        //数据首指针
        char *shmBuf = buf;
        char *tempSrc = NULL;
        char *tempDest = NULL;
        
        //首 > 尾
        if(head >= tail){
            memcpy((void *)&buf[tail],(const void *)item,(size_t)size);
        }
        //尾 < 首
        else{
            int rightSize = qHead->leftSize - tail;
            //右边的空间 < 包的空间
            if(rightSize < *size){
                //包的前面一部分
                memcpy((void *)&buf[tail],(const void *)&item[0],(size_t)rightSize);
                //包的后面一部分
                memcpy((void *)&buf[0],(const void *)&item[rightSize],(size_t)(size-rightSize)) ;       
            }
            //右边的空间 > 包的空间
            else{
                 memcpy((void *)&buf[tail],(const void *)item,(size_t)size);
            }
        }
        tail = (tail + *size) % qHead->leftSize;
        qHead->tail = tail;
        
        return tail;
    }
    
private:
    int getLeftSize(){
        int retSize =0;
        int head = -1;
        int tail = -1;
        
        head = qHead->head;
        tail = qHead->tail;
        //首 == 尾
        if(head == tail){
            retSize = qHead->leftSize;
        }
        //首 >  尾
        else if(head > tail){
            retSize = head - tail;
        }
        //首 <  尾
        else{
            retSize = qHead->leftSize - (head-tail);
        }
        retSize -= BUFF_RESERVE_LENGTH;
        
        return retSize;
        
    }
    int getUsedSize(){
        int usedSize = qHead->leftSize - getLeftSize();
        return usedSize;
    }
    bool isFull(){
        if(getLeftSize() < 1){
            return false;
        }else{
            return true;
        }
    }
    char* buf;
    QHead* qHead;
};


void shm_init(){
    QShm qshm;
    qshm.init(12345,5000);
    qshm.getBuf();
    QSQueue queue = QSQueue(qshm.getBuf());
    char item[100];
    int size = 0;
    memset(item,0,sizeof(item));
    int code = queue.getItem(item,&size);
    printf("code:%d,item:%s,size:%d\n",code,item,size);
    
    if(1) return;
    
    int shmid = -1;
    printf("::%d\n",shmid);
    int pnOutLen = 10;

}
/*
 * 
 */
int main(int argc, char** argv) {
    //shm_init();
    
    printf("LEN:%d\n",sizeof(short));
    return 0;
}

