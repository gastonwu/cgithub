
#include "protocol.h"

#include <stdint.h>
#include <stdio.h>
//#include "dna_endec.h"

#define CHK_RST(x) do{ if (x < 0) return -1; }while(0)

int EncodePkg(void *buf, int buflen, CMsgHeader *header, 
		int body_len, char *body_buf, int ext_len, char *ext_buf,
		char *err_msg)
{
	*err_msg = 0;
	char *_buf = (char *)buf;

	header->total_len = 1+HEADER_LEN+2+body_len+1+ext_len+1;
	CStreamEncoder encoder(buf, buflen);
	encoder.EncodeUInt8(ADSTX);
	int bytes = header->Encode(_buf+1, buflen-1);
	if (bytes != HEADER_LEN)
	{
		sprintf(err_msg, "decode header error %d", bytes);
		return -1;
	}
	encoder.MoveBytes(HEADER_LEN);

	encoder.EncodeNUInt16((uint16_t)body_len);
	encoder.EncodeMem(body_buf, body_len);
	
	encoder.EncodeUInt8((uint8_t)ext_len);
	encoder.EncodeMem(ext_buf, ext_len);

	encoder.EncodeUInt8(ADETX);
	
	return 0;
}

int DecodePkg(const void *buf, int buflen, CMsgHeader *header, 
		int *body_len, char **body_buf, int *ext_len, char **ext_buf,
		char *err_msg)
{
	*err_msg = 0;
	const char *_buf = (const char *)buf;
	if (buflen < (1+HEADER_LEN+2+1+1))
	{	
		sprintf(err_msg, "buflen %d is invalid", buflen);
		return -1;
	}
	if (_buf[0] != ADSTX || _buf[buflen-1] != ADETX)
	{
		sprintf(err_msg, "check bound error %hhu %hhu", _buf[0], _buf[buflen-1]);
		return -1;
	}
	CStreamDecoder decoder(buf, buflen);
	decoder.MoveBytes(1);
	
	int bytes = header->Decode(_buf+1, buflen-1);
	if (bytes != HEADER_LEN)
	{
		sprintf(err_msg, "decode header error %d", bytes);
		return -1;
	}
	if (header->total_len != buflen)
	{
		sprintf(err_msg, "check total_len error %d != %d", (int)header->total_len, buflen);
		return -1;
	}
	decoder.MoveBytes(HEADER_LEN);
	
	uint16_t _body_len;
	decoder.DecodeNUInt16(&_body_len);
	*body_len = _body_len;
	*body_buf = (char *)decoder.GetCurBuffer();
	decoder.MoveBytes(*body_len);

	uint8_t _ext_len;
	decoder.DecodeUInt8(&_ext_len);
	*ext_len = _ext_len;
	*ext_buf = (char *)decoder.GetCurBuffer();
	decoder.MoveBytes(*ext_len);
	
	return 0;
}


CMsgHeader::CMsgHeader()
{
	memset(this, 0, sizeof(*this));
	version = PROTOCOL_VER;
}

CMsgHeader::~CMsgHeader()
{
}

int CMsgHeader::Encode(void *buf, int buflen)
{
	version = PROTOCOL_VER;

	CStreamEncoder encoder(buf, buflen);
	CHK_RST( encoder.EncodeNUInt16(total_len) );
	CHK_RST( encoder.EncodeUInt8(version) );
	CHK_RST( encoder.EncodeUInt8(cmd_type) );
	CHK_RST( encoder.EncodeNUInt32(usr_ip) );
	CHK_RST( encoder.EncodeNUInt16(usr_port) );
	CHK_RST( encoder.EncodeNUInt32(cgi_ip) );
	CHK_RST( encoder.EncodeNUInt16(cgi_port) );
	CHK_RST( encoder.EncodeNUInt32(intf_ip) );
	CHK_RST( encoder.EncodeNUInt16(intf_port) );
	CHK_RST( encoder.EncodeNUInt32(cmd_id) );
	CHK_RST( encoder.EncodeNUInt32(uin) );
	CHK_RST( encoder.EncodeNUInt32(act_id) );
	CHK_RST( encoder.EncodeNUInt32(socket_fd) );
	CHK_RST( encoder.EncodeNUInt32(seq) );
	CHK_RST( encoder.EncodeNUInt32(send_time) );
	CHK_RST( encoder.EncodeNUInt32(flag) );
	CHK_RST( encoder.EncodeMem(reserve, 8) );
	return encoder.GetCurLen();
}

int CMsgHeader::Decode(const void *buf, int buflen)
{
	CStreamDecoder decoder(buf, buflen);
    CHK_RST( decoder.DecodeNUInt16(&total_len) );
    CHK_RST( decoder.DecodeUInt8(&version) );  
    CHK_RST( decoder.DecodeUInt8(&cmd_type) ); 
    CHK_RST( decoder.DecodeNUInt32(&usr_ip) );  
    CHK_RST( decoder.DecodeNUInt16(&usr_port) );
    CHK_RST( decoder.DecodeNUInt32(&cgi_ip) );  
    CHK_RST( decoder.DecodeNUInt16(&cgi_port) );
    CHK_RST( decoder.DecodeNUInt32(&intf_ip) ); 
    CHK_RST( decoder.DecodeNUInt16(&intf_port) );
    CHK_RST( decoder.DecodeNUInt32(&cmd_id) );  
    CHK_RST( decoder.DecodeNUInt32(&uin) );
    CHK_RST( decoder.DecodeNUInt32(&act_id) );  
    CHK_RST( decoder.DecodeNUInt32(&socket_fd) );  
    CHK_RST( decoder.DecodeNUInt32(&seq) );  
	CHK_RST( decoder.DecodeNUInt32(&send_time) );
	CHK_RST( decoder.DecodeNUInt32(&flag) );
    CHK_RST( decoder.DecodeMem(reserve, 8) ); 
	return decoder.GetCurLen();
}

void CMsgHeader::Print()
{
	printf("\t\t---   Show protocol data list  ---\n");
	printf("total_len %hu, version %hhu ... \n", total_len, version);
	printf("cmd_type %hhu...\n", cmd_type);
	printf("usr_ip : %u...\n", usr_ip);
	printf("usr_port : %hu...\n", usr_port);
	printf("cgi_ip : %u...\n", cgi_ip);
	printf("cgi_port : %hu...\n", cgi_port);
	printf("intf_ip : %u...\n", intf_ip);
	printf("intf_port : %u...\n", intf_port);
	printf("cmd_id %u...\n", cmd_id);
	printf("uin %u...\n", uin);
	printf("flag %u...\n", flag);
	printf("act_id %u...\n", act_id);
	printf("seq %u...\n", seq);
	printf("\t\t---            End show              ---\n");	
}



