all: cExample.o cppFile.o
	g++ cExample.o cppFile.o -g -o cppFile
cExample.o:
	g++ cExample.c cExample.h -g -c -o cExample.o
cppFile.o:
	g++ cppFile.c -g -c -o cppFile.o
clean:
	rm -rf *.o cppFile
