// c++实现文件，调用add：cppFile.cpp
extern "C"
{
#include "cExample.h"
}
int main(int argc, char* argv[])
{
	add(2,3);
	return 0;
}
