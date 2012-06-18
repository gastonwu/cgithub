#include "bitmap.cpp"

int main(){
	int million = 1000000;
	int million10 = 10 * million;
	unsigned long billion = 1000 * million;
	unsigned long large_number = 3 * billion;
    BitMap bitmap;
    for(unsigned long i=0;i<large_number;i++){
    	bitmap.set(i);
    }
    printf("exist:%u\n",bitmap.get(large_number-1));
    //bitmap.set(123);

	return 1;
}