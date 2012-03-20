#include "../module.h"
#include "protocol.h"
#include "dna_endec.h"
#include <iostream>
#include <stdio.h>

using namespace std;

int encodeRequest(char* data, unsigned &len)
{
	char err_msg[64];
	char body_buf[1024];
	char send_buf[1024];
	CMsgHeader _msg_header; 
	_msg_header.version = 1;
	_msg_header.cmd_type = REQUEST;
	_msg_header.cmd_id = 2112;
	_msg_header.uin = 221275102;


	//encode body
	CStreamEncoder encoder(body_buf, sizeof(body_buf));
	encoder.EncodeNUInt16LVString("faddrname",strlen("faddrname"));
	encoder.EncodeNUInt16LVString("fext1",strlen("fext1"));
	
	int body_len = encoder.GetCurLen();
	EncodePkg(send_buf, sizeof(send_buf), &_msg_header, body_len, body_buf, 0, NULL,err_msg);

	len = _msg_header.total_len;
	memcpy(data,send_buf,len);
	return 0;
}

int decodeResponse(char* data,unsigned len)
{
	return 0;
}

