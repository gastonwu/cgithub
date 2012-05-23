INC_LIBEVENT       =  

all: server 
server: dna_sharemem.o test2.o
	g++ dna_sharemem.o test2.o -o server
dna_sharemem.o:
	g++ -g -c dna_sharemem.cpp  -o dna_sharemem.o
test2.o:
	g++ -g -c test2.cpp -o test2.o
clean:
	rm -rf *.o server client test test2
