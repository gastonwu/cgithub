#ifndef _MODULE_H_
#define _MODULE_H_

#include<stdlib.h>


int encodeRequest(char* data, unsigned &len);

int decodeResponse(char* data,unsigned len);

#endif
