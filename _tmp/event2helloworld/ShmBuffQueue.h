#ifndef _SHM_BUFF_QUEUE_H_
#define _SHM_BUFF_QUEUE_H_
#include "Shm.h"

#define BUFF_RESERVE_LENGTH 8
class CShmBuffQueue
{

public:
	CShmBuffQueue(char *pShmBuff);
	~CShmBuffQueue();

	//从buff   取数据, 只改变begin偏移
	int GetDataUnit(char *pOut, short *psOutLen);
	//向buff 加数据, 只改变end偏移
	int PutDataUnit(const char *pIn, short sInLen);
    void PrintHead(int i);

private:
	CShmBuffQueue(){}
	int GetLeftSize();
	int GetUsedSize();
	bool IsFull();
	DataHead *m_pDataHead;
	char *m_pBuff;
};
#endif
