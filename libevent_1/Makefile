all: main server2
server2:
	g++ server2.cpp -o server2 -levent
main:
	g++ main.cpp -o libevent_1 -levent
run:
	./libevent_1
clean:
	rm main server2