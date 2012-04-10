#include "./module.h"
//#include "protocol.h"
//#include "dna_endec.h"
#include <iostream>
#include <stdio.h>
#include <string.h>
//
////proto buffer
//#include "../sqlparam.pb.h"
//#include "../result.pb.h"
//#include <iostream>
//#include <fstream>
using namespace std;


int encodeRequest(char* data, unsigned &len)
{
    //protobuf encode & send
//    dbproxy::sqlparam sqlparam;
//    sqlparam.set_id(101);
//    sqlparam.set_sql("select * from test.user where %s=%s limit 2");
//    sqlparam.add_param("1");
//    sqlparam.add_param("1");

    //    sqlparam.set_sql("insert into user(title) value('%s')");
    //    sqlparam.add_param("1");

    //sqlparam.set_sql("update  user set title='111'");
    // sqlparam.add_param("2");

//    string line;
//    sqlparam.SerializeToString(&line);
    
	char send_buf[1024];
	sprintf(send_buf,"%s","haha");
	len = strlen(send_buf);
	memcpy(data,send_buf,len);
	return 0;
}

int decodeResponse(char* data,unsigned len)
{
	// printf("%s\n",data);
	// if(len>0){
	// 	//return 3;
	// 	return 0;
	// }else{
	// 	return 2;
	// }

	return 0;	
}

