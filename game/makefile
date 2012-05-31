all:
server: dna_sharemem.o
	g++ -g -c server.cpp -o server.o
	g++ dna_sharemem.o server.o -o server
client: dna_sharemem.o
	g++ -g -c client.cpp -o client.o
	g++ dna_sharemem.o client.o -o client
dna_sharemem.o:
	g++ -g -c dna_sharemem.cpp  -o dna_sharemem.o
clean:
	rm -rf *.o server server2 client test test2 test3 test4 test5
rmshare:
	ipcrm -M 0x20101230;ipcrm -M 0x30101230;ipcrm -M 0x40101230
	ipcs -m
