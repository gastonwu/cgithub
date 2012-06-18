#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


int main(){
	int million = 1000000;
	int million10 = 10 * million;
	unsigned long billion = 1000 * million;
	unsigned long large_number = 4 * billion;
	bool * int_array[10*100000];

	printf("%u,%u,%u,%u\n",million,million10,billion,large_number);
}