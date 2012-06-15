// Class Name: Multi Level Hash
//
#ifndef __DNA_MULTI_HASH_H__
#define __DNA_MULTI_HASH_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*

int odd_is_prime(int num)
{
	int i;
	int n = (int)sqrt(num);
	for (i=3; i<=n; i+=2)
		if (num % i == 0)
			return 0;
	return 1;
}

int get_primes_under_num(int num, int max_count, int *prime_list)
{
	int count = 0;
	if (num % 2 == 0)
		--num;
	for ( ; num>2; num-=2)
	{
		if (odd_is_prime(num))
		{
			prime_list[count++] = num;
			if (count == max_count)
				break;
		}
	}
	return count;
}

int is_prime(int num)
{
	if (num == 2)
		return 1;
	if (num % 2 == 0)
		return 0;
	return odd_is_prime(num);
}

eg: char *

class CStringHashItem
{
public:
	typedef char * KeyType;
	typedef const char * const ConstKeyType;

	static unsigned int GetUintHash(ConstKeyType & key) { return *(unsigned int *)key; }
	static int * GetRowSizeList()
	{
		static int prime_list[ROW_COUNT] = {299993,299983,299977,.....};
		return prime_list;
	};

	inline ConstKeyType GetKey() const { return name; };
	inline bool SameKey(ConstKeyType & key) const { return strcmp(name, key)==0; };
	inline bool IsEmpty() const { return *name=='\0'; };
	inline void SetEmpty() { *name='\0'; };

public:
	char name[MAX_NAME_LEN+1];
	unsigned int mapping_id;
	char nick[30+1];
}

eg: unsigned long

class CIntHashItem
{
public:
	typedef unsigned long KeyType;
	typedef const unsigned long ConstKeyType;

	static unsigned int GetUintHash(ConstKeyType & key) { reuturn uin; };
	static int * GetRowSizeList();

	inline ConstKeyType GetKey() const { return uin; };
	inline bool SameKey(ConstKeyType & key) const { return key==uin; };
	inline bool IsEmpty() const { return uin==0; } ;
	inline void SetEmpty() { uin=0; };

public:
	unsigned int uin;
	char nick[30+1];
}

typedef CMultiHashCache<CHashItem, HASH_ROW, HASH_COL> CHashTable;

CHashTable *hashtable = new CHashTable;

*/

template<class _DataItemType, size_t _HashRow, size_t _HashCol>
class CMultiHashCache
{
public:
	typedef typename _DataItemType::KeyType KeyType;
	typedef typename _DataItemType::ConstKeyType ConstKeyType;

public:
	inline _DataItemType * GetObjByKey(ConstKeyType & key);
	inline _DataItemType * GetObjByKeyEx(ConstKeyType & key, _DataItemType * &empty_node);
	inline int GetObjIdxByKey(ConstKeyType & key);
	int InsertObj(const _DataItemType & obj);
	int RemoveObjByKey(ConstKeyType & key);
	

public:
	union {
		_DataItemType m_items[_HashRow][_HashCol];
		_DataItemType m_itemlist[_HashRow*_HashCol];
	};
};

template<class _DataItemType, size_t _HashRow, size_t _HashCol>
_DataItemType * CMultiHashCache<_DataItemType,_HashRow,_HashCol>::GetObjByKey(ConstKeyType & key)
{
	unsigned int hash = _DataItemType::GetUintHash(key);
	int *prime_list = _DataItemType::GetRowSizeList();
	unsigned int col;
	_DataItemType *obj;

	for (size_t row=0; row<_HashRow; ++row)
	{
		col = hash % prime_list[row];
		obj = &m_items[row][col];
		if (obj->SameKey(key))
			return obj;
	}
	return NULL;
}

template<class _DataItemType, size_t _HashRow, size_t _HashCol>
_DataItemType * CMultiHashCache<_DataItemType,_HashRow,_HashCol>::GetObjByKeyEx(ConstKeyType & key, _DataItemType * &empty_node)
{
	empty_node = NULL;

	unsigned int hash = _DataItemType::GetUintHash(key);
	int *prime_list = _DataItemType::GetRowSizeList();
	unsigned int col;
	_DataItemType *obj;

	for (size_t row=0; row<_HashRow; ++row)
	{
		col = hash % prime_list[row];
		obj = &m_items[row][col];
		if (!empty_node && obj->IsEmpty())
			empty_node = obj;
		if (obj->SameKey(key))
			return obj;
	}
	return NULL;
}

template<class _DataItemType, size_t _HashRow, size_t _HashCol>
int CMultiHashCache<_DataItemType,_HashRow,_HashCol>::GetObjIdxByKey(ConstKeyType & key)
{
	_DataItemType *obj = GetObjByKey(key);
	return obj ? obj - m_itemlist : -1;
}

template<class _DataItemType, size_t _HashRow, size_t _HashCol>
int CMultiHashCache<_DataItemType,_HashRow,_HashCol>::InsertObj(const _DataItemType & new_obj)
{
	ConstKeyType key = new_obj.GetKey();

	unsigned int hash = _DataItemType::GetUintHash(key);
	int *prime_list = _DataItemType::GetRowSizeList();
	unsigned int col;
	_DataItemType *obj;

	for (size_t row=0; row<_HashRow; ++row)
	{
		col = hash % prime_list[row];
		obj = &m_items[row][col];
		if (obj->IsEmpty() || obj->SameKey(key)) 
		{
			*obj = new_obj;
			return obj - m_itemlist;
		}
	}
	return -1;
}

template<class _DataItemType, size_t _HashRow, size_t _HashCol>
int CMultiHashCache<_DataItemType,_HashRow,_HashCol>::RemoveObjByKey(ConstKeyType & key)
{
	unsigned int hash = _DataItemType::GetUintHash(key);
	int *prime_list = _DataItemType::GetRowSizeList();
	unsigned int col;
	_DataItemType *obj;

	for (size_t row=0; row<_HashRow; ++row)
	{
		col = hash % prime_list[row];
		obj = &m_items[row][col];
		if (obj->SameKey(key))
		{
			obj->SetEmpty();
			return obj - m_itemlist;
		}
	}
	return -1;
}

#endif
