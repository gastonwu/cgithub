/* 
 * File:   main.cpp
 * Author: gaston
 *
 * Created on 2012年6月14日, 下午10:05
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


using namespace std;


class BitMap{
private:
    uint32_t bitLen;
    char * bitmap;
public:
    BitMap(unsigned long len=3200000000ul){
        bitLen = len;
        bitmap = new char[len/8 + 1];
        memset(bitmap,0,len/8+1);
    }
    
    ~BitMap(){
      delete bitmap;
    }
    
    int set(unsigned long num){
        int pos = num / 8;
        int charPos = num % 8;
        bitmap[pos] |= 1 << charPos;

        return 1;
    }
    
    int get(unsigned long num){
        int pos = num / 8;
        int charPos = num % 8;
        
        char ch = bitmap[pos];
        
        int flag = (ch & (1 << charPos) ) >> charPos;
        
        return flag;
    }
    
};

/*
int main() {
    
    BitMap bitmap;
    bitmap.set(123);
    int flag1 = bitmap.get(124);
    int flag2 = bitmap.get(123);
    printf("flag1:%d\tflag2:%d\n",flag1,flag2);

    return 1;
}
*/