//test.cpp

#include <string.h>
#include <stdio.h>
#include "hash_shm.h"
#include <cstdlib>
#include "game.h"

using namespace std;

void test1(){
     hash_shm<char,1000,100> ht(key_t(999));
    double rate=0.0;
//    ht.clear();
    for(int i=0;i<100;i++){
        srand(time(NULL)+i);
        while(true){
            if(ht.insert(rand(),0)==-1)break;
        }
        cout<<ht.getFullRate()<<endl;
        rate+=ht.getFullRate();
        ht.clear();
    }
    cout<<"\n\n\n";
    cout<<rate/100<<endl;   
}

void test2(){

     hash_shm<UsrData,1000,100> ht(key_t(2000));

UsrData sendUser,user2;

memcpy(sendUser.usr_id,"100001",sizeof("100001"));
memcpy(sendUser.usr_pwd,"123456",sizeof("123456"));
memcpy(sendUser.usr_nickname,"Rock",sizeof("Rock"));
    sendUser.age = 19;

     //char* str = "haha";
     ht.insert(2,sendUser);
     ht.insert(3,sendUser);

     int pos = ht.find(2);
     if(pos == 0){
        //printf()
     }
     printf("pos:%d\n",pos);

}


int main()
{
    test2();
}