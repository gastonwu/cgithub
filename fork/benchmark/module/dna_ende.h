/*
*文件名称: dna_endec.h
*文件标识: 
*摘    要: 定义通用编码解码类. 
*
*当前版本: 1.0
*作者:   周健 HoraceZhou
*完成日期: 2006-05-13
*最后更新日期: 2007-07-10
*/

#ifndef __BASIC_ENDEC_HPP_
#define __BASIC_ENDEC_HPP_

#ifdef WIN32
	#include <windows.h>
#else
	#include <netinet/in.h>
#endif

#include <limits>
#include <functional>
#include <string.h>

struct htons_t: std::unary_function<unsigned short, unsigned short>
{
	unsigned short operator()(unsigned short& _X) const
	{
		return htons(_X); 
	}
};

struct ntohs_t: std::unary_function<unsigned short, unsigned short>
{
	unsigned short operator()(unsigned short& _X) const
	{
		return ntohs(_X); 
	}
};

struct htonl_t: std::unary_function<unsigned int, unsigned int>
{
	unsigned int operator()(unsigned int& _X) const
	{
		return htonl(_X); 
	}
};

struct ntohl_t: std::unary_function<unsigned int, unsigned int>
{
	unsigned int operator()(unsigned int& _X) const
	{
		return ntohl(_X); 
	}
};

// --------------------------------------------------------------------------------

// 把value开始的valueLen长的数据，编码到destBuf指向的内存
// 返回 -n 表示destBuf缺少的长度为n，>= 0 表示编码的长度
// destBufLen < 0 表示不需要检验destBuf的长度（destBuf长度无穷大）
int EncodeMem(void *destBuf, int destBufLen, const void *value, size_t valueLen);

// value需要valueLen这么长的数据，如果srcBuf的数据不够长则返回错误
// 返回 -n 表示缺少的长度为n，>= 0 表示编码的长度
// srcBufLen < 0 表示不需要检验srcBuf的长度
int DecodeMem(const void *srcBuf, int srcBufLen, void *value, size_t valueLen);

// --------------------------------------------------------------------------------

template<typename _VTy>
inline int EncodeFixedlenType(void *destBuf, int destBufLen, _VTy value)
{
	return EncodeMem(destBuf, destBufLen, &value, sizeof(_VTy));
}

template<typename _VTy, class _Pt>
inline int EncodeFixedlenType_Fn(void *destBuf, int destBufLen, _VTy value)
{
	value = _Pt()(value);
	return EncodeMem(destBuf, destBufLen, &value, sizeof(_VTy));
}

template<typename _VTy>
inline int DecodeFixedlenType(const void *srcBuf, int srcBufLen, _VTy *value)
{
	return DecodeMem(srcBuf, srcBufLen, value, sizeof(_VTy));
}

template<typename _VTy, class _Pt>
inline int DecodeFixedlenType_Fn(const void *srcBuf, int srcBufLen, _VTy *value)
{
	int result = DecodeMem(srcBuf, srcBufLen, value, sizeof(_VTy));
	if (result > 0)
		*value = _Pt()(*value);
	return result;
}

// --------------------------------------------------------------------------------
// VC6 Bug: 当“模板参数”没有出现在函数的“形参列表”中时函数签名无法区分不同实例。
// 解决方法：加上一个 _LTy *noUse 参数解决，这个参数在函数中是没有用的。

// 功能：把字符串str，以LV（length-value）格式编码到destBuf。字符串的0结尾将不被编码。
// 返回值：被编码的长度。
// 参数：
//     destBuf：目标缓冲区指针
//     destBufLen：目标缓冲区指针长度。当 < 0 表示不需要检验destBuf的长度（destBuf长度无穷大）
//     str：源字符串，必须是0结尾的字符串。当长于maxStrLen（当>0）时，超出的部分被截断，所以调用者需自己判断str长度是否超出。
//     maxStrLen：最多从源字符串编码的长度。

// eg：
// char buf[25];
// EncodeLVString<short>(buf, 25, "abcde", -1)
// 执行后返回7。buf变成 05 00 61 62 63 64 65。其中05 00是short(5)，61 62 63 64 65是chars(abcde)
// 
template<typename _LTy>
int EncodeLVString(void *destBuf, int destBufLen, const char *str, int maxStrLen, _LTy *noUse = NULL)
{
	int realStrLen = (int)strlen(str);

#ifdef max
#undef max
#endif
	if ((size_t)realStrLen > (size_t)std::numeric_limits<_LTy>::max())
		realStrLen = (int)std::numeric_limits<_LTy>::max();

	_LTy strLen = 0; 
	if (((int)maxStrLen >= 0) && (realStrLen > maxStrLen))
		strLen = (_LTy)maxStrLen;
	else
		strLen = (_LTy)realStrLen;

	// 如需要检验长度，则检验长度
	bool checkLen = (destBufLen >= 0);
	if ((checkLen) && ((int)strLen + (int)sizeof(_LTy) > destBufLen))
		return destBufLen - (strLen + sizeof(_LTy));
	
	int codeLen = EncodeFixedlenType<_LTy>(destBuf, checkLen ? destBufLen : -1, strLen);
	if (codeLen < 0)
		return codeLen;
	int codeLen2 = EncodeMem((char *)destBuf+codeLen, checkLen ? destBufLen-codeLen : -1, str, strLen);
	if (codeLen2 < 0)
		return codeLen2;
	return codeLen + codeLen2;
}

template<typename _LTy, class _Pt>
int EncodeLVString_Fn(void *destBuf, int destBufLen, const char *str, int maxStrLen, _LTy *noUse = NULL)
{
	int realStrLen = (int)strlen(str);

#ifdef max
#undef max
#endif
	if ((size_t)realStrLen > (size_t)std::numeric_limits<_LTy>::max())
		realStrLen = (int)std::numeric_limits<_LTy>::max();

	_LTy strLen = 0; 
	if ((maxStrLen >= 0) && (realStrLen > maxStrLen))
		strLen = (_LTy)maxStrLen;
	else
		strLen = (_LTy)realStrLen;

	// 如需要检验长度，则检验长度
	bool checkLen = (destBufLen >= 0);
	if ((checkLen) && ((int)strLen + (int)sizeof(_LTy) > destBufLen))
		return destBufLen - (strLen + sizeof(_LTy));
	
	int codeLen = EncodeFixedlenType_Fn<_LTy, _Pt>(destBuf, checkLen ? destBufLen : -1, strLen); // <-------------
	if (codeLen < 0)
		return codeLen;
	int codeLen2 = EncodeMem((char *)destBuf+codeLen, checkLen ? destBufLen-codeLen : -1, str, strLen);
	if (codeLen2 < 0)
		return codeLen2;
	return codeLen + codeLen2;
}

// 功能：把srcBuf内容，以LV（length-value）格式解码到str。str接受字符串最大长度为maxStrLen（不包括0结尾）。
// 返回值：被解码的长度。
// 参数：
//     srcBuf：源缓冲区指针
//     srcBufLen：源缓冲区指针长度。当 < 0 表示不需要检验srcBuf的长度（srcBuf长度无穷大）
//     str：目的字符串。
//     maxStrLen：str接受的最大长度（不包括0结尾）。

// 注意：
// maxStrLen: 不包括结束符，所以字符串缓冲区应该maxStrLen+1

// eg:
// char str[MAXLEN+1];
// DecodeLVString(strBuf, strBufLen, str, MAXLEN);

template<typename _LTy>
int DecodeLVString(const void *srcBuf, int srcBufLen, char *str, int maxStrLen, _LTy *noUse = NULL)
{
	bool checkLen = (srcBufLen >= 0);
	if ((checkLen) && ((int)sizeof(_LTy) > srcBufLen))
		return srcBufLen - (int)sizeof(_LTy);

	_LTy strLen = 0; 
	int codeLen = DecodeFixedlenType<_LTy>(srcBuf, srcBufLen, &strLen);
	if (codeLen < 0)
		return codeLen;
//	if (strLen < (_LTy)0) // added 20070709
//		return -1;

	if ((maxStrLen >= 0) && ((int)strLen > maxStrLen))
		strLen = maxStrLen;
	if (checkLen) {
		srcBufLen -= codeLen;
		if (srcBufLen < 0)
			return srcBufLen;
	}
	
	srcBuf = (const char *)srcBuf + codeLen;
	int codeLen2 = DecodeMem(srcBuf, srcBufLen, str, strLen);
	if (codeLen2 < 0)
		return codeLen2;
	str[strLen] = 0;
	return codeLen + codeLen2;
}

template<typename _LTy, class _Pt>
int DecodeLVString_Fn(const void *srcBuf, int srcBufLen, char *str, int maxStrLen, _LTy *noUse = NULL)
{
	bool checkLen = (srcBufLen >= 0);
	if ((checkLen) && ((int)sizeof(_LTy) > srcBufLen))
		return srcBufLen - (int)sizeof(_LTy);

	_LTy strLen = 0; 
	int codeLen = DecodeFixedlenType_Fn<_LTy, _Pt>(srcBuf, srcBufLen, &strLen); // <-------------
	if (codeLen < 0)
		return codeLen;
//	if (strLen < (_LTy)0)  // added 20070709
//		return -1;

	if ((maxStrLen >= 0) && ((int)strLen > maxStrLen))
		strLen = maxStrLen;
	if (checkLen) {
		srcBufLen -= codeLen;
		if (srcBufLen < 0)
			return srcBufLen;
	}
	
	srcBuf = (const char *)srcBuf + codeLen;
	int codeLen2 = DecodeMem(srcBuf, srcBufLen, str, strLen);
	if (codeLen2 < 0)
		return codeLen2;
	str[strLen] = 0;
	return codeLen + codeLen2;
}

// --------------- Encode ---------------
#define _EncodeInt8 EncodeFixedlenType<char>
#define _EncodeUInt8 EncodeFixedlenType<unsigned char>

#define _EncodeInt16 EncodeFixedlenType<short>
#define _EncodeUInt16 EncodeFixedlenType<unsigned short>
#define _EncodeNUInt16 EncodeFixedlenType_Fn<unsigned short, htons_t>

#define _EncodeInt32 EncodeFixedlenType<int>
#define _EncodeUInt32 EncodeFixedlenType<unsigned int>
#define _EncodeNUInt32 EncodeFixedlenType_Fn<unsigned int, htonl_t>

#define _EncodeUInt8LVString EncodeLVString<unsigned char>
#define _EncodeUInt16LVString EncodeLVString<unsigned short>
#define _EncodeNUInt16LVString EncodeLVString_Fn<unsigned short, htons_t>
#define _EncodeUInt32LVString EncodeLVString<unsigned int>
#define _EncodeNUInt32LVString EncodeLVString_Fn<unsigned int, htonl_t>

int EncodeBitStr(char *destBuf, unsigned char fromBitPos, const char *bits, int maxBitCount);
// Encode 一个0结尾的字符串
// maxStrLen 不包含0结尾的长度
int EncodeString(void *destBuf, int destBufLen, const char *str, int maxStrLen);

// --------------- Decode ---------------
#define _DecodeInt8 DecodeFixedlenType<char>
#define _DecodeUInt8 DecodeFixedlenType<unsigned char>

#define _DecodeInt16 DecodeFixedlenType<short>
#define _DecodeUInt16 DecodeFixedlenType<unsigned short>
#define _DecodeNUInt16 DecodeFixedlenType_Fn<unsigned short, ntohs_t>

#define _DecodeInt32 DecodeFixedlenType<int>
#define _DecodeUInt32 DecodeFixedlenType<unsigned int>
#define _DecodeNUInt32 DecodeFixedlenType_Fn<unsigned int, ntohl_t>
// new
//#define _DecodeNInt32(srcBuf, srcBufLen, value) _DecodeNInt32(srcBuf, srcBufLen, (unsigned int *)value)

#define _DecodeUInt8LVString DecodeLVString<unsigned char>
#define _DecodeUInt16LVString DecodeLVString<unsigned short>
#define _DecodeNUInt16LVString DecodeLVString_Fn<unsigned short, ntohs_t>
#define _DecodeUInt32LVString DecodeLVString<unsigned int>
#define _DecodeNUInt32LVString DecodeLVString_Fn<unsigned int, ntohl_t>

int DecodeBitStr(const char *srcBuf, unsigned char fromBitPos, char *bits, int maxBitCount);
// Decode 一个0结尾的字符串
// maxStrLen 不包含0结尾的长度
int DecodeString(const void *srcBuf, int srcBufLen, char *str, int maxStrLen);

// --------------- CStreamEncoder ---------------
//*
class CStreamHandler
{
public:
	enum {
		MAX_BOOKMARK = 10,
	};
	struct stBookmark 
	{
		unsigned int offset;
		unsigned char bitpos;
	};

public:
	CStreamHandler();
	CStreamHandler(void *Buf, int BufLen, unsigned char BitPos = 0);
	virtual ~CStreamHandler();

public:
	void SetBuffer(void *Buf, int BufLen, unsigned char BitPos = 0);
	void * GetBuffer(int *bufLen = NULL, unsigned char *bitPos = NULL);
	void * GetCurBuffer(int *leftBufLen = NULL, unsigned char *bitPos = NULL);
	unsigned int GetCurLen(unsigned char *bitLen = NULL);

	// MoveBytes 不会执行align，所以如需要align，需在调用函数前执行AlignBits;
	// 返回 0 成功，否则失败
	int MoveBytes(int byteCount);
	// 返回 0 成功，否则失败
	int MoveBits(int bitCount);
	
	// 返回移动的bit数
	int AlignBits();
	
protected:
	// 根据索引获取书签
	stBookmark * GetBookmark(int idx);
	// 如果idx=-1, 表示自动分配。返回索引
	int SetBookmark(int idx, unsigned int offset, unsigned char bitpos = 0);
	// 把当前位置加入到书签
//	stBookmark * AddCurToBookmark(int idx);

	char *m_Buf;
	int m_BufLen;  // 当前还[剩余]多少byte, 或者说是m_destBuf的长度
	unsigned char m_BitPos;  // 记录当前pos的bit指向那一位 0-7

	char *m_curBuf;  // 当前指针
	int m_curBufLen; // 当前还[剩余]多少byte, 如果m_curBitPos>0, 则其实只有(m_curBufLen-1)字节加(8-m_curBitPos)位可用, 但仍然表示为m_curBufLen
	unsigned char m_curBitPos;  // 记录当前pos的bit指向那一位 0-7
	
	bool m_checkLen;

private:
	stBookmark m_bookmarks[MAX_BOOKMARK];
};
//*/

// --------------- CStreamEncoder ---------------
class CStreamEncoder : public CStreamHandler
{
public:
	CStreamEncoder();
	CStreamEncoder(void *Buf, int BufLen, unsigned char BitPos = 0);

public:
	int EncodeMem(const void *value, size_t valueLen);
	
	int EncodeInt8(char value);
	int EncodeUInt8(unsigned char value);

	int EncodeInt16(short value);
	int EncodeUInt16(unsigned short value);
	int EncodeNUInt16(unsigned short value);

	int EncodeInt32(int value);
	int EncodeUInt32(unsigned int value);
	int EncodeNUInt32(unsigned int value);

	// maxStrLen 是不包含0结尾的长度
	int EncodeNullTermString(const char *str, int maxStrLen);
	int EncodeUInt8LVString(const char *str, int maxStrLen);
	int EncodeUInt16LVString(const char *str, int maxStrLen);
	int EncodeNUInt16LVString(const char *str, int maxStrLen);
	int EncodeUInt32LVString(const char *str, int maxStrLen);
	int EncodeNUInt32LVString(const char *str, int maxStrLen);

	// 返回被编码的bit数
	// bitCount: bit的长度，必须>=0
	//eg: EncodeBits("101111011100", 12)
	int EncodeBitStr(const char *bits, int bitCount);
	
public:
	int EncodeChar(char value) {
		return EncodeInt8(value);
	}
	int EncodeShort(short value) {
		return EncodeNUInt16((unsigned short)value);
	}
	int EncodeInt(int value) {
		return EncodeNUInt32((unsigned int)value);
	}
	int EncodeString(const char *str, int maxStrLen) {
		return EncodeNUInt16LVString(str, maxStrLen);
	}
};

// --------------- CStreamDecoder ---------------
class CStreamDecoder : public CStreamHandler
{
public:
	CStreamDecoder();
	CStreamDecoder(const void *Buf, int BufLen, unsigned char BitPos = 0);;

public:
	int DecodeMem(void *value, size_t valueLen);
	
	int DecodeInt8(char *value);
	int DecodeUInt8(unsigned char *value);

	int DecodeInt16(short *value);
	int DecodeUInt16(unsigned short *value);
	int DecodeNUInt16(unsigned short *value);

	int DecodeInt32(int *value);
	int DecodeUInt32(unsigned int *value);
	int DecodeNUInt32(unsigned int *value);

	// maxStrLen 如果>0，表示不包含0结尾的最大长度，所以str的缓冲区长度需要至少是maxStrLen+1
	int DecodeNullTermString(char *str, int maxStrLen);
	int DecodeUInt8LVString(char *str, int maxStrLen);
	int DecodeUInt16LVString(char *str, int maxStrLen);
	int DecodeNUInt16LVString(char *str, int maxStrLen);
	int DecodeUInt32LVString(char *str, int maxStrLen);
	int DecodeNUInt32LVString(char *str, int maxStrLen);

	// 返回被解码的bit数
	// bitCount: bit的长度，必须>=0
	// 注意：不会在bits字符串结尾自动添加0
	//eg: DecodeBits(bits, 12); bits[12] = 0;
	int DecodeBitStr(char *bits, int bitCount);

public:
	int DecodeChar(char *value) {
		return DecodeInt8(value);
	}
	int DecodeShort(short *value) {
		return DecodeNUInt16((unsigned short *)value);
	}
	int DecodeInt(int *value) {
		return DecodeNUInt32((unsigned int *)value);
	}
	int DecodeString(char *str, int maxStrLen) {
		return DecodeNUInt16LVString(str, maxStrLen);
	}
};

#endif
