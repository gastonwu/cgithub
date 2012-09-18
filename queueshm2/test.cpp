#include <iostream>  
#include <stdio.h>
#include <string.h>

template<class _DataItemType, size_t length>
class CMultiHashCache
{
	public:
		inline _DataItemType * GetObjByKey(int id);
		int InsertObj(const _DataItemType & obj);
	public:
		_DataItemType _items[length];
};

template<class _DataItemType, size_t length>
_DataItemType * CMultiHashCache<_DataItemType,length>::GetObjByKey(int id)
{
	return NULL;
}

template<class _DataItemType, size_t length>
int CMultiHashCache<_DataItemType,length>::InsertObj(const _DataItemType & new_obj)
{
	return -1;
}

struct Msg{
	char line[100];
}typedef Msg;

typedef CMultiHashCache<Msg, 10> HashUser;
HashUser* hu;

int main(){
	hu->GetObjByKey(10);
	Msg msg;
	memset(&msg,0,sizeof(msg));
	memcpy(&msg.line,"abc",sizeof("abc"));

	hu->InsertObj(msg);

	printf("ha\n");
	return -1;
}
