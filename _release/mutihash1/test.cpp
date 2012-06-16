//test.cpp

#include <string.h>
#include "hash_shm.h"
#include <cstdlib>

using namespace std;
int main()
{
    hash_shm<int,1000,100> ht(key_t(999));
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