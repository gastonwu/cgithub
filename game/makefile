
all: 
test2: dna_sharemem.o test2.o
	g++ dna_sharemem.o test2.o -o test2
test3: dna_sharemem.o test3.o
	g++ dna_sharemem.o test3.o -o test3
dna_sharemem.o:
	g++ -g -c dna_sharemem.cpp  -o dna_sharemem.o
test2.o:
	g++ -g -c test2.cpp -o test2.o
test3.o:
	g++ -g -c test3.cpp -o test3.o
clean:
	rm -rf *.o server client test test2
