#include <stdio.h>
#include "dna_multi_hash.h"
#include "dna_sharemem.h"


#define HASH_ROW 7
#define HASH_COL 100000


static int prime_list[HASH_ROW] = {99991, 99989, 99971, 99961, 99929, 99923, 99907};

class CHashItem
{
public:
    typedef unsigned long KeyType;
    typedef const unsigned long ConstKeyType;

    static unsigned int GetUintHash(const KeyType & key)
    {
        return key;
    }

    static int * GetRowSizeList()
    {
        return prime_list;
    }

    const KeyType & GetKey() const 
    {
        return m_val;
    }

    bool SameKey(const KeyType & key) const
    {
        return key == m_val;
    }

    bool IsEmpty() const
    {
        return m_val==0;
    }

    void SetEmpty()
    {
        m_val = 0;
    }
public:
    unsigned long m_val;
};

//typedef unsigned int conv_func_t(char *, char *);
typedef CMultiHashCache<CHashItem, HASH_ROW, HASH_COL> HashTable;


int main(){
    struct stShmInfo shm_info;
    shm_info.key = 0x20101230;
    shm_info.size = sizeof(HashTable) * 2;
    shm_info.shm_id = -1;
    shm_info.shm_addr = NULL;
    shm_info.log[0] = 0;

    bool new_created;
    if (GetShm(&shm_info, &new_created) == NULL) {
        fprintf(stderr, shm_info.log);
        return -1;
    }

    HashTable *ht = (HashTable *)shm_info.shm_addr;
    int val = 12;

    CHashItem obj;
    obj.m_val = val;
    int ret = ht->InsertObj(obj);
    obj.m_val =13;
    ret = ht->InsertObj(obj);
    if (ret < 0) {
        fprintf(stderr, "InsertObj error %d\n", ret);
        //continue;
    }
    
    CHashItem *obj2 = ht->GetObjByKey(val);
    printf(":::%d\n",obj2->m_val);

    //CHashItem *p = NULL;
    for (int r=0; r<HASH_ROW; ++r)
    {
        for (int c=0; c<HASH_COL; ++c){
            CHashItem *p = &(ht->m_items[r][c]);
            if(p->m_val) {
                printf("->%u\n", p->m_val);
                //++count;
                //printf("%s\n", get_ip_str_n(ht->m_items[r][c].m_val));
            }
        }
    }

    //HashTable.insert()
    printf("ha\n");
}