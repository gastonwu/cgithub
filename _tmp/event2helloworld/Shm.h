#ifndef _SHM_H_
#define _SHM_H_
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdio.h>

struct DataHead {
    int iSize;  //剩余空间大小
    int iBegin; //读指针，位置
    int iEnd;   //写指钍，位置
    int iOffset;//偏移量，每一个单位数据的长度
};
//
//struct DataUnit {
//    int iLen;
//    char *pData;
//};

enum SHM_RET {
    SHM_ERROR = -1,
    CREATE_SUCC = 0,
    ATTACH_SUCC = 1,
};

class CShm {
public:


    CShm();
    CShm(size_t iShmSize, key_t iKey);
    ~CShm();
    int Init(size_t iShmSize, key_t iKey);

    char *GetShmBuff() {
        return m_pShmBuff;
    }
    int Detach();
    int Delete();

protected:
    size_t m_iShmSize;
    key_t m_iShmKey;
    char *m_pShmBuff;
    int m_iShmID;

};

#endif
