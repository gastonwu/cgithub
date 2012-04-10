INC_PROTOBUF       = -I/usr/local/protobuf/include -I./ 
PROTOBUF_LIBFILE   = /usr/local/protobuf/lib/libprotobuf.a
LIB_SQLPARAM	   = ./libsqlparam.a
LIB_RESULT		   = ./libresult.a
LIB_PROTOBUF       = $(PROTOBUF_LIBFILE) -lpthread 
INC_MYSQL		   = -I/usr/include/mysql
LIB_MYSQL		   = -L/usr/lib/mysql/ `mysql_config --cflags --libs`
COMMON_PATH		   = $(LIB_SQLPARAM) $(LIB_RESULT) $(LIB_PROTOBUF) $(INC_PROTOBUF) $(INC_MYSQL) $(LIB_MYSQL)


all: server client
server:
	g++ -g server.cpp -o server -levent
client:
	g++ -g client.cpp -o client -levent
client.pb: libsqlparam libresult
	g++ -g client.pb.cpp -o client.pb -levent $(COMMON_PATH)
clean:
	rm -rf server client client.pb server.pb *.a *.pb.h *.pb.cc
gencode:
	protoc -I=./ --cpp_out=./ sqlparam.proto
	protoc -I=./ --cpp_out=./ result.proto
	
libsqlparam: gencode
	g++ $(INC_PROTOBUF) $(LIB_PROTOBUF) -c sqlparam.pb.cc
	ar crv libsqlparam.a sqlparam.pb.o
libresult: gencode
	g++ $(INC_PROTOBUF) $(LIB_PROTOBUF) -c result.pb.cc
	ar crv libresult.a result.pb.o