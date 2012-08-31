#include "Shm.h"
#include <errno.h>
#include <string.h>

CShm::CShm() {
    m_iShmID = -1;
    m_iShmKey = -1;
    m_iShmSize = 0;
    m_pShmBuff = NULL;
}

CShm::CShm(size_t iShmSize, key_t iKey) {
    Init(iShmSize, iKey);
}

CShm::~CShm() {
    Detach();
}

int CShm::Init(size_t iShmSize, key_t iKey) {
    /*
        成功返回共享内存的标识符；不成功返回-1，errno储存错误原因。
        EINVAL           参数size小于SHMMIN或大于SHMMAX。
        EEXIST           预建立key所致的共享内存，但已经存在。
        EIDRM            参数key所致的共享内存已经删除。
        ENOSPC        超过了系统允许建立的共享内存的最大值(SHMALL )。
        ENOENT        参数key所指的共享内存不存在，参数shmflg也未设IPC_CREAT位。
        EACCES        没有权限。
        ENOMEM       核心内存不足。

     */

    int iRet = SHM_ERROR;
    m_iShmID = shmget(iKey, iShmSize, IPC_CREAT | IPC_EXCL | 0666);
    if (m_iShmID < 0) {
        if (errno != EEXIST) {
            printf("Alloc share memory failed, %s \n", strerror(errno));
            return SHM_ERROR;
        }
        //可能已经存在,尝试 Attach
        if ((m_iShmID = shmget(iKey, iShmSize, 0666)) < 0) {

            printf("Attach to share memory %d failed, %s. Now try to touch it \n", iKey, strerror(errno));

            m_iShmID = shmget(iKey, 0, 0666);
            if (m_iShmID < 0) {
                printf("error, touch shm failed, %s. \n", strerror(errno));
                return SHM_ERROR;
                ;
            } else {
                printf("remove the exist share memory %d \n", m_iShmID);
                if (shmctl(m_iShmID, IPC_RMID, NULL)) {
                    printf("error, remove share memory failed, %s \n", strerror(errno));
                    return SHM_ERROR;
                    ;
                }

                //重新创建
                m_iShmID = shmget(iKey, iShmSize, IPC_CREAT | IPC_EXCL | 0666);
                if (m_iShmID < 0) {
                    printf("error, create memory failed, %s\n", strerror(errno));
                    return SHM_ERROR;
                }
                iRet = CREATE_SUCC;

            }
        } else {
            printf("attach to share memory %d succeed.\n ", iKey);
            iRet = ATTACH_SUCC;
        }

    } else {

        printf("create share memory block, key = %08X, id = %d, size = %d \n", iKey, m_iShmID, iShmSize);
        iRet = CREATE_SUCC;
    }

    //访问
    if ((m_pShmBuff = (char *) shmat(m_iShmID, NULL, 0)) == NULL) {

        printf("error, access share memory %d failed.\n ", m_iShmID);
        return SHM_ERROR;
    }

    m_iShmSize = iShmSize;
    m_iShmKey = iKey;

    //初始化共享内存头
    DataHead stDataHead;
    if (iRet == CREATE_SUCC) {

        stDataHead.iBegin = 0;
        stDataHead.iEnd = 0;
        stDataHead.iOffset = sizeof (DataHead);
        stDataHead.iSize = iShmSize - sizeof (DataHead);
        memcpy(m_pShmBuff, (char*) &stDataHead, sizeof (DataHead));
    } else {
        memcpy((char*) &stDataHead, m_pShmBuff, sizeof (DataHead));
        printf("DataHead:\n\tiSize:%u\n\tiBegin:%u\n\tiEnd:%u\n\tiOffset:%u\n", stDataHead.iSize, stDataHead.iBegin, stDataHead.iEnd, stDataHead.iOffset);

    }
    return iRet;
}

int CShm::Detach() {
    int iRet = 0;
    if (m_pShmBuff != NULL) {
        iRet = shmdt(m_pShmBuff);

        m_pShmBuff = NULL;
    }

    
    return iRet;
}

int CShm::Delete() {
    int iRet = 0;
    if (m_pShmBuff != NULL) {
        iRet = shmctl(m_iShmID, IPC_RMID, 0);

        m_pShmBuff = NULL;
    }

    return iRet;
}

