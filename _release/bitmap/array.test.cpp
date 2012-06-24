#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


int main(){
	int million = 1000000;//1百万
	int million10 = 10 * million;//千万
	unsigned long billion = 1000 * million;//10亿
	unsigned long large_number = 4 * billion;
	int * int_array[million10];

	printf("%u,%u,%u,%u\n",million,million10,billion,large_number);
}