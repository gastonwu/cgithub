INC_LIBEVENT       = -I/usr/local/libevent/include -L/usr/local/libevent/lib

all: server client
server:
	g++ -g  $(INC_LIBEVENT) server.cpp -o server -levent
client:
	g++ -g  $(INC_LIBEVENT) client.cpp -o client -levent
clean:
	rm -rf server client