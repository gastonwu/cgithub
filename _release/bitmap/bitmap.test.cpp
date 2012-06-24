#include "bitmap.cpp"

int main(){
	int million = 1000000;//百万
	int million10 = 10 * million;//千万
	unsigned long billion = 1000 * million;//10亿
	unsigned long large_number = 1 * billion;
    BitMap bitmap;
    for(unsigned long i=0;i<large_number;i++){
    	bitmap.set(i);
    }
    printf("exist:%u\n",bitmap.get(large_number-1));
    printf("not exist:%u\n",bitmap.get(large_number+2));
    //bitmap.set(123);

	return 1;
}