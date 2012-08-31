#include "ShmBuffQueue.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

CShmBuffQueue::CShmBuffQueue(char *pShmBuff)
{

	m_pDataHead = (DataHead*)pShmBuff;
	m_pBuff = pShmBuff + sizeof(DataHead);
}
CShmBuffQueue::~CShmBuffQueue()
{
	m_pBuff = NULL;
	m_pDataHead = NULL;
}
void CShmBuffQueue::PrintHead(int i=0)
{
  //printf("---------------------------end:%u\n",m_pDataHead->iEnd);
  printf("[PrintHead:%d]\tsize:%u\t\toffset:%u\t\tbegin:%u\t\tend:%u\n",i,m_pDataHead->iSize,m_pDataHead->iOffset,m_pDataHead->iBegin,m_pDataHead->iEnd);
  //printf("DataHead:\n\tiSize:%u\n\tiBegin:%u\n\tiEnd:%u\n\tiOffset:%u\n", m_pDataHead->iSize, m_pDataHead->iBegin,m_pDataHead->iEnd, m_pDataHead->iOffset);
}

/**
 * 取数据
 * @param pOut
 * @param pnOutLen
 * @return 
 */
int CShmBuffQueue::GetDataUnit(char *pOut, short *pnOutLen)
{
	int iLeftSize = 0;

	int iBegin = -1;
	int iEnd = -1;

	//参数判断
	if((NULL == pOut) || (NULL == pnOutLen))
	{
		return -1;
	}

	if( m_pDataHead->iOffset <= 0 || m_pDataHead->iSize <= 0 )
	{
		return -1;
	}

	//取首、尾
	iBegin = m_pDataHead->iBegin;
	iEnd   = m_pDataHead->iEnd;


	//空
	if( iBegin == iEnd )
	{
		*pnOutLen = 0;
		return 0;
	}

	//剩余缓冲大小,小于包长度字节数,错误返回
	iLeftSize = GetLeftSize();
	if( iLeftSize < sizeof(short) )
	{
		//重置首尾
		*pnOutLen = 0;
		m_pDataHead->iBegin = 0;
		m_pDataHead->iEnd = 0;
		return -3;
	}

	char *pbyCodeBuf = m_pBuff;
	char *pTempSrc = NULL;
	char *pTempDst = NULL;

	pTempDst = (char *)pnOutLen;
	pTempSrc = (char *)&pbyCodeBuf[0];

	//包长度编码
	for(int i = 0; i < sizeof(short); i++ )
	{
		pTempDst[i] = pTempSrc[iBegin];
		iBegin = (iBegin+1) % m_pDataHead->iSize; 
	}

	//数据包长度非法
	if(((*pnOutLen) > GetUsedSize())  || (*pnOutLen < 0))
	{
		*pnOutLen = 0;
		m_pDataHead->iBegin = 0;
		m_pDataHead->iEnd = 0;		
		return -3;
	}

	pTempDst = pOut;

	//首小于尾
	if(iBegin < iEnd )
	{
		memcpy((void *)pTempDst, (const void *)&pTempSrc[iBegin], (size_t)(*pnOutLen));
	}
	else
	{
		//首大于尾且出现分段，则分段拷贝
		int iRightLeftSize = m_pDataHead->iSize - iBegin;
		if( iRightLeftSize < *pnOutLen)
		{
			
			memcpy((void *)pTempDst, (const void *)&pTempSrc[iBegin], iRightLeftSize);
			pTempDst += iRightLeftSize;
			memcpy((void *)pTempDst, (const void *)&pTempSrc[0], (size_t)(*pnOutLen - iRightLeftSize));
		}
		//否则，直接拷贝
		else
		{
			memcpy((void *)pTempDst, (const void *)&pTempSrc[iBegin], (size_t)(*pnOutLen));
		}
	}

	//变更begin
	iBegin = (iBegin + (*pnOutLen)) % m_pDataHead->iSize;

	m_pDataHead->iBegin = iBegin;

	return iBegin;
}
int CShmBuffQueue::PutDataUnit(const char *pIn, short nInLen)
{
	int iLeftSize = 0;
	int iBegin = -1;
	int iEnd = -1;


	//参数判断
	if((NULL == pIn) || (nInLen <= 0))
	{
		return -1;
	}
	
	if( m_pDataHead->iOffset <= 0 || m_pDataHead->iSize <= 0 )
	{
		return -1;
	}


	//首先判断是已满
	if(IsFull())	
	{
		return -2;
	}

	//取首、尾
	iBegin = m_pDataHead->iBegin;
	iEnd   = m_pDataHead->iEnd;

	//缓冲区异常判断处理
	if( iBegin < 0 || iBegin >= m_pDataHead->iSize
		|| iEnd < 0 || iEnd >= m_pDataHead->iSize )
	{

		//重置
		m_pDataHead->iBegin = 0;
		m_pDataHead->iEnd = 0;
		

		return -3;
	}

	//剩余缓冲大小小于新来的数据,溢出了,返回错误
	iLeftSize = GetLeftSize();
//printf("::%d-leftSize:%d\n",sizeof(short),iLeftSize);
	if((int)(nInLen + sizeof(short)) > iLeftSize)
	{
		return -2;
	}


	//数据首指针
	char *pbyCodeBuf = m_pBuff;

	char *pTempSrc = NULL;
	char *pTempDst = NULL;

	
	pTempDst = &pbyCodeBuf[0];
	pTempSrc = (char *)&nInLen;
	//包的长度编码
	for(int i = 0; i < sizeof(nInLen); i++ )
	{
		pTempDst[iEnd] = pTempSrc[i];
		iEnd = (iEnd+1) % m_pDataHead->iSize;
	}

	//首大于尾
	if( iBegin > iEnd )
	{
		memcpy((void *)&pbyCodeBuf[iEnd], (const void *)pIn, (size_t)nInLen);
	}
	else
	{
		//首小于尾,本包长大于右边剩余空间,需要分两段循环放到buff存放
		if((int)nInLen > (m_pDataHead->iSize - iEnd))
		{
			//右边剩余buff
			int iRightLeftSize = m_pDataHead->iSize - iEnd;		
			memcpy((void *)&pbyCodeBuf[iEnd], (const void *)&pIn[0], (size_t)iRightLeftSize );
			memcpy((void *)&pbyCodeBuf[0],(const void *)&pIn[iRightLeftSize], (size_t)(nInLen - iRightLeftSize));
		}
		//右边剩余buff够了，直接put
		else
		{
			memcpy((void *)&pbyCodeBuf[iEnd], (const void *)&pIn[0], (size_t)nInLen);
		}
	}

	//更新尾偏移
	iEnd = (iEnd + nInLen) % m_pDataHead->iSize;
	m_pDataHead->iEnd = iEnd;

	return iEnd;
}

/**
 * 队列是否已满
 * @return bool
 */
bool CShmBuffQueue::IsFull()
{
	int iLeftSize = 0;
	iLeftSize = GetLeftSize();

	if( iLeftSize > 0 )
	{
		return false;
	}
	else
	{
		return false;
	}
}

/**
 * 取得剩余队列空间大小
 * @return int
 */
int CShmBuffQueue::GetLeftSize()
{
	int iRetSize = 0;
	int iBegin = -1;
	int iEnd = -1;

	iBegin = m_pDataHead->iBegin;	
	iEnd = m_pDataHead->iEnd;

	//首尾相等
	if(iBegin == iEnd)
	{
		iRetSize = m_pDataHead->iSize;
	}
	//首大于尾
	else if(iBegin > iEnd)
	{
		iRetSize = iBegin - iEnd;
	}
	//首小于尾
	else
	{
		iRetSize = m_pDataHead->iSize - iEnd + iBegin;
	}
    int beforeSize = iRetSize;
	//最大长度减去预留部分长度，保证首尾不会相接
	iRetSize -= BUFF_RESERVE_LENGTH;
    //printf("\t\tbefore:%d\treserve:%d\tafter:%d,[%d]\n",beforeSize,BUFF_RESERVE_LENGTH,iRetSize,(iBegin-iEnd));
	return iRetSize;
}

/**
 * 取得使用过的队列空间大小
 * @return 
 */
int CShmBuffQueue::GetUsedSize()
{
	int iLeftSize = GetLeftSize();
	if(iLeftSize > 0)
	{
		return m_pDataHead->iSize - iLeftSize;
	}
	else
	{
		return m_pDataHead->iSize;
	}
}

