INC_PROTOBUF       = -I/usr/local/protobuf/include -I./ 
PROTOBUF_LIBFILE   = /usr/local/protobuf/lib/libprotobuf.a
LIB_SQLPARAM	   = ../libsqlparam.a
LIB_RESULT		   = ../libresult.a
LIB_PROTOBUF       = $(PROTOBUF_LIBFILE) -lpthread 
INC_MYSQL		   = -I/usr/include/mysql
LIB_MYSQL		   = -L/usr/lib/mysql/ `mysql_config --cflags --libs`
COMMON_PATH	= $(LIB_SQLPARAM) $(LIB_RESULT) $(LIB_PROTOBUF) $(INC_PROTOBUF) $(INC_MYSQL) $(LIB_MYSQL)


DEF= -DCOMPILE_DATE=\"$(DATE)\" -DCOMPILE_TIME=\"$(TIME)\" -DVERSION=\"$(VER)\"

PROJ_ROOT =./benchmark
INC = -I$(PROJ_ROOT)/ ${COMMON_PATH}
LIB = -lpthread -ldl -lz -lm 

C_FLAGS =  -O0  $(DEF) -g  -Wall  -pg
#C_FLAGS =  -O0  $(DEF) -g -fomit-frame-pointer -Wall  
CXX             = g++ 
RANLIB          = ranlib
AR              = ar
SRCS = $(wildcard *.cpp module/*.cpp )
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
TARGET = benchmark 
OK = \\e[1m\\e[32m OK \\e[m
FAILURE = \\e[1m\\e[31m FAILURE \\e[m
all:$(TARGET)

$(TARGET):$(OBJS)
		@echo -ne Linking $(TARGET) ... 
		@$(CXX) $(C_FLAGS)  $(INC) -o $@ $^ $(LIB) && echo  -e $(OK) || echo -e $(FAILURE)
		@rm -f *.o
%.o:%.cpp
		@echo -ne Compiling $<  ... 
		@$(CXX) $(C_FLAGS)  $(INC) -c -o $@ $< && echo  -e $(OK) || echo -e $(FAILURE)

	
clean:
	@rm -f *.o 
	@rm -f ./$(TARGET)
install:clean all
	strip $(TARGET) 
