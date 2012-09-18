/* 
 * File:   queue_shm.h
 * Author: gaston
 *
 * Created on 2012年9月17日, 上午10:44
 */

#ifndef QUEUE_SHM_H
#define	QUEUE_SHM_H

#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>

template<class _DataItemType, int length>
class QueueShm {
public:
    QueueShm();
    virtual ~QueueShm();
    int pop(_DataItemType &item);
    int push(const _DataItemType &item);
    int getTest();
    bool isFull();
    bool isEmpty();
    void printStatus();
    
public:
    _DataItemType _items[length];
    int debug;
private:
    int head;
    int tail;


};

template<class _DataItemType, int length>
QueueShm<_DataItemType, length>::QueueShm() {
    head = 0;
    tail = 0;
}

template<class _DataItemType, int length>
QueueShm<_DataItemType, length>::~QueueShm() {
}

template<class _DataItemType, int length>
int QueueShm<_DataItemType, length>::pop(_DataItemType &item) {
    if (isEmpty()) {
        return -1;
    }
    head = (head + 1) % length;
    //if(debug) printf("\t");printStatus();
    memcpy(&item, &_items[head], sizeof (_items[head]));

    return 0;
}

template<class _DataItemType, int length>
int QueueShm<_DataItemType, length>::push(const _DataItemType &item) {
    //printf("full:%d,tail:%d\n",isFull(),tail);
    if (isFull()) {
        return -1;
    }
     //if(1) printf("\n\t[1]");printStatus();
   tail = (tail + 1) % length;
     //if(debug) printf("\t[2]");printStatus();
    _items[tail] = item;
     //if(debug) printf("\t[3]");printStatus();
   return 0;
}

template<class _DataItemType, int length>
int QueueShm<_DataItemType, length>::getTest() {
    return 1;
}

template<class _DataItemType, int length>
bool QueueShm<_DataItemType, length>::isFull() {

    return (head == (tail + 1) % length) ? true : false;
}

template<class _DataItemType, int length>
bool QueueShm<_DataItemType, length>::isEmpty() {
    return (head == tail) ? true : false;
}

template<class _DataItemType, int length>
void QueueShm<_DataItemType, length>::printStatus() {
    printf("head:%d tail:%d\n",head,tail);
}

class QShm {
public:

    ~QShm() {
        if (buf != NULL) {
            shmdt(buf);
        }
    }

    void init(int key, int size) {
        int shmId = shmget(key, size, IPC_CREAT | 600);
        if (shmId < 0) {
            printf("QShm.init:shmget error\n");
            return;
        }
        buf = (char *) shmat(shmId, NULL, 0);
        if (buf == NULL) {
            printf("QShm.init:shmat error\n");
            return;
        }

    }

    char* getBuf() {
        return buf;
    }
private:
    char* buf;
    size_t size;
    key_t key;
};

#endif	/* QUEUE_SHM_H */

