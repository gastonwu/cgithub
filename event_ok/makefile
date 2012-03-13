all: timer_server
server2:
	g++ -g server2.c -o server2 -levent
timer_server:
	g++ -g timer_server.c -o timer_server -levent
run:
	./timer_server
clean:
	rm timer_server