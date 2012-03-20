
//#include "stdafx.h"
#include "dna_endec.h"
#include <assert.h>

// --------------- Utils ---------------
namespace {
 	static const unsigned char LeftFillOneMaskChar[9] = {0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
 	static const unsigned char LeftFillZeroMaskChar[9] = {0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01, 0x00};
	static const unsigned char OneMaskChar[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
	static const unsigned char ZeroMaskChar[8] = {0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe};
} // end of namespace

#ifdef WIN32
namespace {
size_t strnlen(const char *s, size_t maxlen)
{
	const char *p = s;
	const char *pend = s + maxlen;

	while (*p && p<pend)
		++p;

	return (size_t)(p - s);
}
} // end of namespace
#else
#include <string.h>
#endif

// 从leftOffset这个bit开始从左向右填充bitCount个1，其余位都是0
// leftOffset: [0,7]
// bitCount: [0,8]
// eg: GetSerialOneMask(1,3) = 0x70 (01110000)
unsigned char GetSerialOneMask(unsigned char leftOffset, unsigned char bitCount)
{
	assert(leftOffset<=7);
	assert(leftOffset<=(8-leftOffset));

	if (bitCount == 0)
		return 0;

	unsigned char result = 0xff;
	result <<= (8-bitCount);
	result >>= leftOffset;
	return result;
}

// fromBitPos 0-7
int EncodeBitStr(char *destBuf, unsigned char fromBitPos, const char *bits, int maxBitCount)
{
	if (fromBitPos > 7)
		return -1;
	if (maxBitCount < 0)
		maxBitCount = strlen(bits);

	int bitCount = 0;
	while (*bits && bitCount < maxBitCount)
	{
		if (*bits == '1') {
			*destBuf |= OneMaskChar[fromBitPos];
		}
		else if (*bits == '0') {
			*destBuf &= ZeroMaskChar[fromBitPos];
		}
		else {
			++bits;
			continue;
		}

		++bits;
		++bitCount;
		if (++fromBitPos == 8) {
			fromBitPos = 0;
			++destBuf;
		}
	}

	return bitCount;
}

//*
//todo
// 不对bits追加0结尾
int DecodeBitStr(const char *srcBuf, unsigned char fromBitPos, char *bits, int maxBitCount)
{
	if (maxBitCount < 0)
		return -1;

	if (fromBitPos > 7)
		return -1;

	int bitCount = 0;
	while (bitCount < maxBitCount)
	{
		if (*srcBuf & OneMaskChar[fromBitPos])
			*bits = '1';
		else
			*bits = '0';

		++bits;
		++bitCount;
		if (++fromBitPos == 8) {
			fromBitPos = 0;
			++srcBuf;
		}
	}

	return bitCount;
}
//*/

// Encode 一个0结尾的字符串
// maxStrLen 不包含0结尾的长度
int EncodeNullTermString(void *destBuf, int destBufLen, const char *str, int maxStrLen)
{
	// destBufLen>=0 表示需要检查长度，因为至少要存放一个结束符0，所以至少要1个字节。
	if (destBufLen == 0)
		return -1;

	int strLen = (int)strlen(str);
	if ((maxStrLen >= 0) && (strLen > maxStrLen))
		strLen = maxStrLen;
	
	int codeLen = EncodeMem((char *)destBuf, destBufLen < 0 ? -1 : destBufLen-1, str, strLen);
	if (codeLen < 0)
		return codeLen;

	((char *)destBuf)[strLen] = 0;
	return codeLen + 1;
}

// Decode 一个0结尾的字符串
// maxStrLen 不包含0结尾的长度
int DecodeNullTermString(const void *srcBuf, int srcBufLen, char *str, int maxStrLen)
{
	bool checkLen = (srcBufLen >= 0);
 	if ((checkLen) && (sizeof(char) > (size_t)srcBufLen)) //至少要有一个0结尾
 		return srcBufLen - sizeof(char);

	int strLen = 0;
	if (checkLen) {
		int max_to_find = (maxStrLen >= 0 && maxStrLen < srcBufLen) ? maxStrLen+1 : srcBufLen;
		strLen = strnlen((char *)srcBuf, (size_t)max_to_find);
		if (strLen == max_to_find)
			return -1;
/*
		char *p= (char *)srcBuf;
		char *pend = (char *)srcBuf;
		if (maxStrLen >= 0 && maxStrLen < srcBufLen)
			pend += maxStrLen;
		else
			pend += srcBufLen;

		while (*p && p<pend)
			++p;

		if (p == pend) // 没有在srcBuf找到0结尾，则返回错误
			return -1;

		strLen = p - (char *)srcBuf;
*/
	}
	else
		strLen = strlen((char *)srcBuf);

	if ((maxStrLen >= 0) && (strLen > maxStrLen))
		return maxStrLen - strLen;

	memcpy(str, srcBuf, strLen);
	str[strLen] = 0;
	return strLen + 1;
}


// --------------- Encode (<--) ---------------
// 把指向value的valuelen个字节编码到destBuf中
int EncodeMem(void *destBuf, int destBufLen, const void *value, size_t valueLen)
{
	if ((destBufLen >= 0) && ((size_t)destBufLen < valueLen))
		return destBufLen - valueLen;

	memcpy(destBuf, value, valueLen);
	return valueLen;
}

// --------------- Decode (-->) ---------------
// 把指向srcBuf解码到value中valueLen个字节
int DecodeMem(const void *srcBuf, int srcBufLen, void *value, size_t valueLen)
{
	if ((srcBufLen >= 0) && ((size_t)srcBufLen < valueLen)) //如果不够填满valueLen这么长，返回错误
		return srcBufLen - valueLen;

	memcpy(value, srcBuf, valueLen);
	return valueLen;
}

// --------------- CStreamHandler ---------------

CStreamHandler::CStreamHandler()
{
	memset(m_bookmarks, 0, sizeof(m_bookmarks));
	SetBuffer(NULL, 0, 0);
}

CStreamHandler::CStreamHandler(void *Buf, int BufLen, unsigned char BitPos)
{
	memset(m_bookmarks, 0, sizeof(m_bookmarks));
	SetBuffer(Buf, BufLen, BitPos);
}

CStreamHandler::~CStreamHandler()
{
}

void CStreamHandler::SetBuffer(void *Buf, int BufLen, unsigned char BitPos)
{
	m_Buf = (char *)Buf;
	m_BufLen = BufLen;
	m_BitPos = BitPos % 8;

	m_curBuf = m_Buf;
	m_curBufLen = m_BufLen;
	m_curBitPos = m_BitPos;

	m_checkLen = (m_BufLen >= 0);
}

void * CStreamHandler::GetBuffer(int *bufLen, unsigned char *bitPos)
{
	if (bufLen)
		*bufLen = m_BufLen;
	if (bitPos)
		*bitPos = m_BitPos;
	return m_Buf;
}

void * CStreamHandler::GetCurBuffer(int *leftBufLen, unsigned char *bitPos)
{
	if (leftBufLen)
		*leftBufLen = m_curBufLen;
	if (bitPos)
		*bitPos = m_curBitPos;
	return m_curBuf;
}

unsigned int CStreamHandler::GetCurLen(unsigned char *bitLen)
{
	if (bitLen)
		*bitLen = m_curBitPos;
	return m_curBuf - m_Buf;
}

int CStreamHandler::MoveBytes(int byteCount)
{
	if (byteCount == 0) 
		return 0;
	
	char *newPos = m_curBuf+byteCount;
	if (newPos < m_Buf)
		return -1;
	if ( (m_checkLen) && (newPos+(m_curBitPos?1:0) > m_Buf+m_BufLen) )
		return -1;

	m_curBuf = newPos; 
	if (m_checkLen) 
		m_curBufLen -= byteCount; 

	return 0;
}

int CStreamHandler::MoveBits(int bitCount)
{
	if (bitCount == 0)
		return 0;
	
	int totalBitCount = m_curBitPos + bitCount;
	int byteCount = totalBitCount / 8;
	// 防止 totalBitCount 是负数情况下产生错误
	int newBitPos = ((totalBitCount % 8) + 8) % 8; // 0-7

	char *newPos = m_curBuf+byteCount;
	// 如果向反方向移动
	if (bitCount < 0) {
		// totalBitCount < 0 表示向反方向移过了byte的边界
		if (totalBitCount < 0 && newBitPos > 0) {
			--byteCount;
			--newPos;
		}
		// 向反方向移动，需要检查开始的边界
		if (newPos < m_Buf)
			return -1;
	}
	// 如果向正方向移动
	else if (m_checkLen) {
		// 向正方向移动，需要检查结束的边界
		if ( newPos+(newBitPos?1:0) > m_Buf+m_BufLen )
			return -1;
	}

	// 如果需要移动字节的位置 ...
	if (byteCount) {
		m_curBuf = newPos; 
		if (m_checkLen) 
			m_curBufLen -= byteCount; 
	}

	m_curBitPos = (unsigned char)newBitPos;
	return 0;
}

CStreamHandler::stBookmark * CStreamHandler::GetBookmark(int idx)
{
	if (idx < 0 || idx >= MAX_BOOKMARK)
		return NULL;
	return m_bookmarks + idx;
}

int CStreamHandler::SetBookmark(int idx, unsigned int offset, unsigned char bitpos /*= 0*/)
{
	if (idx < 0 || idx >= MAX_BOOKMARK)
		return -1;

	stBookmark *bookmark = m_bookmarks + idx;
	bookmark->offset = offset;
	bookmark->bitpos = bitpos;
	return idx;
}


int CStreamHandler::AlignBits()
{
	if (m_curBitPos == 0)
		return 0;
	int bitCount = 8 - m_curBitPos;
	if (MoveBits(bitCount) < 0)
		return -1;
	return bitCount;
}

// --------------- CStreamEncoder ---------------

CStreamEncoder::CStreamEncoder(): CStreamHandler()
{
}

CStreamEncoder::CStreamEncoder(void *Buf, int BufLen, unsigned char BitPos): CStreamHandler(Buf, BufLen, BitPos)
{
}

#define MOVE_POINTER 	\
	do { \
		if (codeLen >= 0) { \
			m_curBuf += codeLen; \
			if (m_checkLen) \
				m_curBufLen -= codeLen; \
		} \
	} while(0)

int CStreamEncoder::EncodeMem(const void *value, size_t valueLen)
{
	AlignBits();
	int codeLen = ::EncodeMem(m_curBuf, m_curBufLen, value, valueLen);
	MOVE_POINTER;
	return codeLen;
}

int CStreamEncoder::EncodeInt8(char value) 
{
	AlignBits();
	int codeLen = _EncodeInt8(m_curBuf, m_curBufLen, value);
	MOVE_POINTER;
	return codeLen;
}

int CStreamEncoder::EncodeUInt8(unsigned char value)
{
	AlignBits();
	int codeLen = _EncodeUInt8(m_curBuf, m_curBufLen, value);
	MOVE_POINTER;
	return codeLen;
}

int CStreamEncoder::EncodeInt16(short value)
{
	AlignBits();
	int codeLen = _EncodeInt16(m_curBuf, m_curBufLen, value);
	MOVE_POINTER;
	return codeLen;
}

int CStreamEncoder::EncodeUInt16(unsigned short value)
{
	AlignBits();
	int codeLen = _EncodeUInt16(m_curBuf, m_curBufLen, value);
	MOVE_POINTER;
	return codeLen;
}

int CStreamEncoder::EncodeNUInt16(unsigned short value)
{
	AlignBits();
	int codeLen = _EncodeNUInt16(m_curBuf, m_curBufLen, value);
	MOVE_POINTER;
	return codeLen;
}

int CStreamEncoder::EncodeInt32(int value)
{
	AlignBits();
	int codeLen = _EncodeInt32(m_curBuf, m_curBufLen, value);
	MOVE_POINTER;
	return codeLen;
}

int CStreamEncoder::EncodeUInt32(unsigned int value)
{
	AlignBits();
	int codeLen = _EncodeUInt32(m_curBuf, m_curBufLen, value);
	MOVE_POINTER;
	return codeLen;
}

int CStreamEncoder::EncodeNUInt32(unsigned int value)
{
	AlignBits();
	int codeLen = _EncodeNUInt32(m_curBuf, m_curBufLen, value);
	MOVE_POINTER;
	return codeLen;
}

int CStreamEncoder::EncodeUInt8LVString(const char *str, int maxStrLen)
{
	AlignBits();
	int codeLen = _EncodeUInt8LVString(m_curBuf, m_curBufLen, str, maxStrLen);
	MOVE_POINTER;
	return codeLen;
}

int CStreamEncoder::EncodeUInt16LVString(const char *str, int maxStrLen)
{
	AlignBits();
	int codeLen = _EncodeUInt16LVString(m_curBuf, m_curBufLen, str, maxStrLen);
	MOVE_POINTER;
	return codeLen;
}

int CStreamEncoder::EncodeNUInt16LVString(const char *str, int maxStrLen)
{
	AlignBits();
	int codeLen = _EncodeNUInt16LVString(m_curBuf, m_curBufLen, str, maxStrLen);
	MOVE_POINTER;
	return codeLen;
}

int CStreamEncoder::EncodeUInt32LVString(const char *str, int maxStrLen)
{
	AlignBits();
	int codeLen = _EncodeUInt32LVString(m_curBuf, m_curBufLen, str, maxStrLen);
	MOVE_POINTER;
	return codeLen;
}

int CStreamEncoder::EncodeNUInt32LVString(const char *str, int maxStrLen)
{
	AlignBits();
	int codeLen = _EncodeNUInt32LVString(m_curBuf, m_curBufLen, str, maxStrLen);
	MOVE_POINTER;
	return codeLen;
}

int CStreamEncoder::EncodeNullTermString(const char *str, int maxStrLen)
{
	AlignBits();
	int codeLen = ::EncodeNullTermString(m_curBuf, m_curBufLen, str, maxStrLen);
	MOVE_POINTER;
	return codeLen;
}

int CStreamEncoder::EncodeBitStr(const char *bits, int bitCount)
{
	if (bitCount == 0)
		return 0;

// 	if (bitCount < 0)
// 		return -1;
// 	if (m_curBitPos > 7)
// 		return -1;
// 	if (m_curBufLen == 0 && m_curBitPos > 0)
// 		return -1;
	assert(! (bitCount < 0));
	assert(! (m_curBitPos > 7));
	assert(! (m_curBufLen == 0 && m_curBitPos > 0));

	if (m_checkLen) {
		int leftBitCount = m_curBufLen * 8 - m_curBitPos;
// 		if (leftBitCount < 0)
// 			return -1;
		assert(leftBitCount >= 0);

		if (leftBitCount < bitCount)
			return leftBitCount - bitCount;
	}

	int codeBitLen = ::EncodeBitStr(m_curBuf, m_curBitPos, bits, bitCount);
	if (codeBitLen >= 0)
		if (MoveBits(codeBitLen) < 0)
			return -1;
	return codeBitLen;
}

// --------------- CStreamDecoder ---------------
CStreamDecoder::CStreamDecoder(): CStreamHandler()
{
}

CStreamDecoder::CStreamDecoder(const void *Buf, int BufLen, unsigned char BitPos): CStreamHandler((void *)Buf, BufLen, BitPos)
{
}

int CStreamDecoder::DecodeMem(void *value, size_t valueLen)
{
	AlignBits();
	int codeLen = ::DecodeMem(m_curBuf, m_curBufLen, value, valueLen);
	MOVE_POINTER;
	return codeLen;
}

int CStreamDecoder::DecodeInt8(char *value)
{
	AlignBits();
	int codeLen = _DecodeInt8(m_curBuf, m_curBufLen, value);
	MOVE_POINTER;
	return codeLen;
}

int CStreamDecoder::DecodeUInt8(unsigned char *value)
{
	AlignBits();
	int codeLen = _DecodeUInt8(m_curBuf, m_curBufLen, value);
	MOVE_POINTER;
	return codeLen;
}

int CStreamDecoder::DecodeInt16(short *value)
{
	AlignBits();
	int codeLen = _DecodeInt16(m_curBuf, m_curBufLen, value);
	MOVE_POINTER;
	return codeLen;
}

int CStreamDecoder::DecodeUInt16(unsigned short *value)
{
	AlignBits();
	int codeLen = _DecodeUInt16(m_curBuf, m_curBufLen, value);
	MOVE_POINTER;
	return codeLen;
}

int CStreamDecoder::DecodeNUInt16(unsigned short *value)
{
	AlignBits();
	int codeLen = _DecodeNUInt16(m_curBuf, m_curBufLen, value);
	MOVE_POINTER;
	return codeLen;
}

int CStreamDecoder::DecodeInt32(int *value)
{
	AlignBits();
	int codeLen = _DecodeInt32(m_curBuf, m_curBufLen, value);
	MOVE_POINTER;
	return codeLen;
}

int CStreamDecoder::DecodeUInt32(unsigned int *value)
{
	AlignBits();
	int codeLen = _DecodeUInt32(m_curBuf, m_curBufLen, value);
	MOVE_POINTER;
	return codeLen;
}

int CStreamDecoder::DecodeNUInt32(unsigned int *value)
{
	AlignBits();
	int codeLen = _DecodeNUInt32(m_curBuf, m_curBufLen, value);
	MOVE_POINTER;
	return codeLen;
}

int CStreamDecoder::DecodeUInt8LVString(char *str, int maxStrLen)
{
	AlignBits();
	int codeLen = _DecodeUInt8LVString(m_curBuf, m_curBufLen, str, maxStrLen);
	MOVE_POINTER;
	return codeLen;
}

int CStreamDecoder::DecodeUInt16LVString(char *str, int maxStrLen)
{
	AlignBits();
	int codeLen = _DecodeUInt16LVString(m_curBuf, m_curBufLen, str, maxStrLen);
	MOVE_POINTER;
	return codeLen;
}

int CStreamDecoder::DecodeNUInt16LVString(char *str, int maxStrLen)
{
	AlignBits();
	int codeLen = _DecodeNUInt16LVString(m_curBuf, m_curBufLen, str, maxStrLen);
	MOVE_POINTER;
	return codeLen;
}

int CStreamDecoder::DecodeUInt32LVString(char *str, int maxStrLen)
{
	AlignBits();
	int codeLen = _DecodeUInt32LVString(m_curBuf, m_curBufLen, str, maxStrLen);
	MOVE_POINTER;
	return codeLen;
}

int CStreamDecoder::DecodeNUInt32LVString(char *str, int maxStrLen)
{
	AlignBits();
	int codeLen = _DecodeNUInt32LVString(m_curBuf, m_curBufLen, str, maxStrLen);
	MOVE_POINTER;
	return codeLen;
}

int CStreamDecoder::DecodeNullTermString(char *str, int maxStrLen)
{
	AlignBits();
	int codeLen = ::DecodeNullTermString(m_curBuf, m_curBufLen, str, maxStrLen);
	MOVE_POINTER;
	return codeLen;
}


int CStreamDecoder::DecodeBitStr(char *bits, int bitCount)
{
 	if (bitCount == 0)
 		return 0;

// 	if (bitCount < 0)
// 		return -1;
// 	if (m_curBitPos > 7)
// 		return -1;
// 	if (m_curBufLen == 0 && m_curBitPos > 0)
// 		return -1;
	assert(! (bitCount < 0));
	assert(! (m_curBitPos > 7));
	assert(! (m_curBufLen == 0 && m_curBitPos > 0));

	if (m_checkLen) {
		int leftBitCount = m_curBufLen * 8 - m_curBitPos;
// 		if (leftBitCount < 0)
// 			return -1;
		assert(leftBitCount >= 0);

		if (leftBitCount < bitCount)
			return leftBitCount - bitCount;
	}

	int codeBitLen = ::DecodeBitStr(m_curBuf, m_curBitPos, bits, bitCount);
	if (codeBitLen >= 0)
		if (MoveBits(codeBitLen) < 0)
			return -1;
	return codeBitLen;
}










