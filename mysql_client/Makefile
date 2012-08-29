INCLUDES = -I/usr/include/mysql/
LIBS = -L/usr/lib/mysql -lmysqlclient #-lm -lsocket -lnsl

all:
	g++ $(INCLUDES) $(LIBS) main.cpp -o main
clean:
	rm -rf main