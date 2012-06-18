ALL: array.test bitmap.test
bitmap:
	g++ -g -c bitmap.cpp -o bitmap.o
array.test:
	g++ array.test.cpp -o array.test
bitmap.test: bitmap
	g++ -g -c bitmap.test.cpp -o bitmap.test.o
	g++ bitmap.o bitmap.test.o -o bitmap.test
clean:
	rm -rf bitmap array.test bitmap.test *.o
