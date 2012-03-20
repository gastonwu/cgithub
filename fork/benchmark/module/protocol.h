#ifndef __PROTOCAL_H__
#define __PROTOCAL_H__

#include <stdint.h>

#define HEADER_LEN 58

#define PROTOCOL_VER 1
#define ADSTX 0x4
#define ADETX 0x5

enum {
	REQUEST  = 1,
	RESPONSE = 2,
	NOTIFY   = 3,
};

#define FLAG_TRACE   0x00000001

class CMsgHeader
{
public:
	uint16_t total_len;
	uint8_t  version;
	uint8_t  cmd_type;
	uint32_t usr_ip;
	uint16_t usr_port;
	uint32_t cgi_ip;
	uint16_t cgi_port;
	uint32_t intf_ip;
	uint16_t intf_port;
	uint32_t cmd_id;
	uint32_t uin;
	uint32_t act_id;
	uint32_t socket_fd;
	uint32_t seq;
	uint32_t send_time;
	uint32_t flag;
	uint8_t  reserve[8];
	
public:
	CMsgHeader();
	~CMsgHeader();
	
	int Encode(void *buf, int buflen);
	int Decode(const void *buf, int buflen);
	void Print();
};

/*
typedef struct{
	uint8_t cStx;
	CMsgHeader stTranPkgHeadAPI;
	uint16_t wBodyLen;
	char body_buf[0];
	uint_t cExt_len;
	char Ext_data[0];	
	uint8_t cEtx;
} TranPkg_API;
*/

int DecodePkg(const void *buf, int buflen, CMsgHeader *header, 
			int *body_len, char **body_buf, int *ext_len, char **ext_buf,
			char *err_msg);

int EncodePkg(void *buf, int buflen, CMsgHeader *header, int body_len, 
			char *body_buf, int ext_len, char *ext_buf,char *err_msg);

#endif

