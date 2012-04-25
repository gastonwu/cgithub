INC_PROTOBUF       = -I/usr/local/protobuf/include -I./ 
PROTOBUF_LIBFILE   = /usr/local/protobuf/lib/libprotobuf.a
INC_LIBEVENT       = -I/usr/local/libevent/include -L/usr/local/libevent/lib
LIB_SQLPARAM	   = ./libsqlparam.a
LIB_RESULT		   = ./libresult.a
LIB_PROTOBUF       = $(PROTOBUF_LIBFILE) -lpthread 

# INC_MYSQL		   = -I/usr/include/mysql
# LIB_MYSQL		   = -L/usr/lib/mysql/ `mysql_config --cflags --libs`
INC_MYSQL		   = -I/usr/local/mysql/include
LIB_MYSQL		   = -L/usr/local/mysql/lib -lmysqlclient


COMMON_PATH	= $(LIB_SQLPARAM) $(LIB_RESULT) $(LIB_PROTOBUF) $(INC_LIBEVENT) $(INC_PROTOBUF) $(INC_MYSQL) $(LIB_MYSQL)


all: server client server.pb client.pb
server:
	g++ -g server.cpp -o server -levent $(COMMON_PATH)
client:
	g++ -g client.cpp -o client -levent $(COMMON_PATH)
client.pb: 
	g++ -g client.pb.cpp -o client.pb -levent $(COMMON_PATH)
server.pb: 
	g++ -g server.pb.cpp -o server.pb -levent $(COMMON_PATH)
clean:
	rm -rf server client client.pb server.pb *.a *.pb.h *.pb.cc *.pb.o *.dSYM
protocode: libsqlparam libresult
gencode:
	protoc -I=./ --cpp_out=./ sqlparam.proto
	protoc -I=./ --cpp_out=./ result.proto
	
libsqlparam: gencode
	g++ $(INC_PROTOBUF) $(LIB_PROTOBUF) -c sqlparam.pb.cc
	ar crv libsqlparam.a sqlparam.pb.o
libresult: gencode
	g++ $(INC_PROTOBUF) $(LIB_PROTOBUF) -c result.pb.cc
	ar crv libresult.a result.pb.o