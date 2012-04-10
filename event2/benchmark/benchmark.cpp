#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <signal.h>
#include <sys/time.h>
#include <iostream>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <getopt.h>
#include "module.h"

using namespace std;

#define MAX_RECV_BUF_LEN 4*1024
#define MAX_SEND_BUF_LEN 4*1024
#define MAX_FAIL_COUNT 1000

#define ENONE 0
#define READABLE 1
#define WRITEABLE 2

#define CONNECT_TIMEOUT 3

typedef enum 
{
	PROTOCOL_TCP = 1,
	PROTOCOL_UDP
}PROTOCOL;

typedef int funcProc(void* );
int createClient();
int sendToServer(void* );
int recvFromServer(void* );

typedef struct 
{
	int stop;
	int epfd;
	struct epoll_event* events;
}epollLoop;

typedef struct
{
	int fd;
	int mask;
	uint64_t latency;
	uint64_t startTime;
	unsigned recvBufLen;
	unsigned sendBufLeft;
	unsigned sendBufLen;
	char recvBuf[MAX_RECV_BUF_LEN + 1]; 
	char sendBuf[MAX_SEND_BUF_LEN + 1]; 
	funcProc* rFuncProc;
	funcProc* wFuncProc;
}client;

typedef struct
{
	unsigned totalRequests;
	unsigned doneRequests;
	uint64_t startTime;
	uint64_t endTime;
	int keepalive;
	unsigned liveClientNum;
	unsigned needClientNum;
	unsigned short port;
	unsigned short protocol; //1 for tcp(default),2 for udp
	const char* host;
	uint64_t* latency;
	epollLoop* el;
	client* clients;
}benchConfig;
benchConfig config;


void ignoreSignal()
{
	signal(SIGTSTP,SIG_IGN); 
	signal(SIGHUP,  SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGSTOP, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
}

uint64_t msTime()
{
	struct timeval tv;
	uint64_t msec;

	gettimeofday(&tv,NULL);
	msec = ((long)tv.tv_sec*1000);
	msec += tv.tv_usec/1000;
	return msec;
}

uint64_t usTime()
{
	struct timeval tv;
	uint64_t usec;

	gettimeofday(&tv, NULL);
	usec = ((uint64_t)tv.tv_sec)*1000000;
	usec += tv.tv_usec;
	return usec;
}

void initConfig()
{
	config.totalRequests = 50;
	config.doneRequests = 0;
	config.port = 5113;
	config.startTime = 0;
	config.endTime = 0;
	config.keepalive = 0;
	config.liveClientNum = 0;
	config.needClientNum = 5;
	config.host = "127.0.0.1";
	config.protocol = PROTOCOL_TCP;
	config.latency = NULL;
	config.el = NULL;
	config.clients = NULL;
}

struct option longopts[] = {
	{ "concurrency", required_argument, NULL, 'c' },
	{ "requests", required_argument, NULL, 'n' },
	{ "host", required_argument, NULL, 'H' },
	{ "port", required_argument, NULL, 'p' },
	{ "keepalive", no_argument, NULL, 'k' },
	{ "protocol", no_argument, NULL, 'u' },
	{ "help", no_argument, NULL, 'h' },
	{ 0, 0, 0, 0},
};

void parseOptions(int argc, char **argv) {
	int opt = 0;
	while ((opt = getopt_long(argc, argv, ":c:n:H:p:kuh", longopts, NULL)) != -1)
	{
		switch (opt) {
			case 'c':
				config.needClientNum = atoi(optarg);
				break;
			case 'n':
				config.totalRequests = atoi(optarg);
				break;
			case 'H':
				config.host = optarg;
				break;
			case 'p':
				config.port = atoi(optarg);
				break;
			case 'k':
				config.keepalive = 1;
				break;
			case 'u':
				config.protocol = PROTOCOL_UDP;	
				break;
			default:
				cout<<"Usage: benchmark [-H <host>] [-p <port>] [-c <clients>] [-n <requests]> [-u]"<<endl;
				cout<<" -H <host> Server ip (default 127.0.0.1)"<<endl;
				cout<<" -p <port> Server port (default 5113)"<<endl;
				cout<<" -c <clients> Number of parallel connections (default 5)"<<endl;
				cout<<" -n <requests> Total number of requests (default 50)"<<endl;
				cout<<" -k keep alive or reconnect (default is reconnect)"<<endl;
				cout<<" -u benchmark with udp protocol(tcp is default)"<<endl;
				exit(1);
			}
	}
}


epollLoop* createEpollLoop(int maxClients)
{
	epollLoop *el;
	el = (epollLoop*)malloc(sizeof(*el));
	if(!el)
	  return NULL;

	el->epfd = epoll_create(1024);
	el->stop = 1;
	el->events = new epoll_event[maxClients];

	return el;
}

int addEvent(client* c,int mask,funcProc* func,benchConfig* conf)
{
	struct epoll_event e;
	int op = c->mask == ENONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD; 
	e.events = 0;
	e.data.u64 = 0;
	e.data.fd = c->fd; 
	c->mask |= mask; /* Merge old events */

	if(c->mask & READABLE)
	{   
		e.events |= EPOLLIN;

	}   
	if(c->mask & WRITEABLE)
	{   
		e.events |= EPOLLOUT;
	}   

	if(mask & WRITEABLE)
	{   
		c->wFuncProc = func;
	}   
	else
	{   
		c->rFuncProc = func;
	}   

	return epoll_ctl(conf->el->epfd,op,c->fd,&e);
}

int delEvent(client* c,int mask,benchConfig* conf)
{
	c->mask = c->mask & (~mask);
	struct epoll_event e;
	e.events = 0;
	e.data.u64 = 0; /* avoid valgrind warning */
	e.data.fd = c->fd;

	if (c->mask & READABLE) 
	{
		e.events |= EPOLLIN;
	}
	if (c->mask & WRITEABLE)
	{
		e.events |= EPOLLOUT;
	}

	int op = c->mask == ENONE ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;

	return epoll_ctl(conf->el->epfd,op,c->fd,&e);
}

void closeConnection(benchConfig* conf,int fd)
{
	close(fd);
	conf->liveClientNum--;
	struct epoll_event e;
	e.events = 0;
	epoll_ctl(conf->el->epfd, EPOLL_CTL_DEL, fd, &e);
}


void resetClient(client* c)
{
	delEvent(c,WRITEABLE,&config);
	delEvent(c,READABLE,&config);
	addEvent(c,WRITEABLE,sendToServer,&config);
	
	//memset(c->recvBuf,0,sizeof(c->recvBuf));
	c->recvBufLen = 0;
	c->sendBufLeft = c->sendBufLen;
}

void clientDone(client* c)
{
	if(config.doneRequests == config.totalRequests)
	{
		config.el->stop = 0;
		return;
	}
	if(config.keepalive)
	{
		resetClient(c);
	}
	else
	{
		closeConnection(&config,c->fd);
		createClient();
	}
}

int recvFromServer(void* cl)
{
	client* c = (client*)cl;
	if(c->latency == 0)
	{
		c->latency = usTime() - c->startTime;
	}

	
	while(1)
	{
		int received = read(c->fd,c->recvBuf,sizeof(c->recvBuf)-c->recvBufLen);
		if(received < 0)
		{
			if(errno == EINTR)
			{
				continue;
			}   
			else if(errno == EWOULDBLOCK || errno == EAGAIN)
			{
				return 1;
			}
			else
			{
				closeConnection(&config,c->fd);
				createClient();
				return -1;
			}
		}
		else if(received == 0)
		{
			//peer closed
			closeConnection(&config,c->fd);
			createClient();
			return -1;
		}
		else
		{
			c->recvBufLen += received;
			break;
		}
	}

	//parse data
	int ret = decodeResponse(c->recvBuf,c->recvBufLen);
	if(ret < 0)
	{
		//clientDone(c);
		closeConnection(&config,c->fd);
		createClient();
		return -1;
	}
	else if(ret == 1)
	{
		//it is not a complete packet
		return 0;
	}

	if(config.doneRequests < config.totalRequests)
	{
		config.latency[config.doneRequests++] = c->latency;
	}

	clientDone(c);
	return 0;
}

int sendToServer(void* cl)
{
	client* c = (client*)cl;
	c->startTime = usTime();
	int sended;
	while(1)
	{   
		sended = write(c->fd,c->sendBuf,c->sendBufLen);
		if(sended <= 0 ) 
		{   
			if (errno == EINTR)
			{   
				continue;
			}   
			return -1;
		}   
		break;
	}

	c->sendBufLeft -= sended;
	if(c->sendBufLeft == 0)
	{   
		delEvent(c,WRITEABLE,&config);
		addEvent(c,READABLE,recvFromServer,&config);
	}   

	return 0;
}

int setNonBlock(int fd)
{
	int flag;
	if((flag = fcntl(fd, F_GETFL, 0)) == -1)
	{
		//log
		return -1;
	}
	flag |= O_NONBLOCK;
	if(fcntl(fd, F_SETFL, flag) == -1)
	{
		//log
		return -1;
	}
	return 0;

}

int setTcpNoDelay(int fd)
{
	int yes = 1;
	if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &yes,sizeof(yes)) == -1) 
	{
		//log
		return -1;
	}
	return 0;
}

int createClient()
{
	int fd;
	if(config.protocol == PROTOCOL_UDP)
	{
		fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	}
	else
	{
		fd = socket(AF_INET, SOCK_STREAM, 0);
	}

	if(fd == -1)
	{
		return -1;
	}

	//set noblock
	if(setNonBlock(fd) < 0 )
	{
		return -1;
	}

	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET; 
	addr.sin_port = htons(config.port);
	addr.sin_addr.s_addr= inet_addr(config.host);
	if(connect(fd, (struct sockaddr *)&addr,sizeof(addr)) == -1)
	{
		//log
		if(errno == EINPROGRESS)
		{
			/*this is ok*/
		}
		else
		{
			close(fd);
			return -1;
		}
	}
	if(config.protocol == PROTOCOL_TCP && setTcpNoDelay(fd) < 0)
	{
		close(fd);
		return -1;
	}


	client* c = config.clients + fd%config.needClientNum;
	c->fd = fd;
	c->mask = ENONE;
	c->latency = 0;
	memset(c->sendBuf,0,sizeof(c->sendBuf));
	memset(c->recvBuf,0,sizeof(c->recvBuf));
	
	//parse cmd
	if(encodeRequest(c->sendBuf,c->sendBufLen) < 0)
	{
		//log
		close(fd);
		return -1;
	}
	c->sendBufLeft = c->sendBufLen;
	
	if(addEvent(c,WRITEABLE,sendToServer,&config) < 0)
	{
		//log
		close(fd);
		return -1;
	}

	config.liveClientNum++;	
	return 0;
}



void createNeedClients()
{
	while(config.liveClientNum < config.needClientNum)
	{
		if(createClient() < 0)
		{
			//log
			cout<<"create client faild"<<endl;
			exit(1);
		}

	}
	
}

int compareLatency(const void *a, const void *b) 
{
	return (*(uint64_t*)a)-(*(uint64_t*)b);
}

void showReport()
{
	uint64_t totalTime = config.endTime - config.startTime;
	float reqpersec = (float)config.doneRequests/((float)totalTime/1000);

	if(config.keepalive == 1)
	{
		cout<<"keepAlive is open"<<endl;
	}
	cout<<config.needClientNum<<" parallel clients"<<endl;
	cout<<config.doneRequests<<" completed in "<<totalTime/1000<<" seconds"<<endl;

	unsigned i, curlat = 0;
	float perc;
	qsort(config.latency,config.doneRequests,sizeof(uint64_t),compareLatency);
	for (i = 0; i < config.doneRequests; i++) {
		if (config.latency[i]/1000 != curlat || i == (config.doneRequests-1)) {
			curlat = config.latency[i]/1000;
			perc = ((float)(i+1)*100)/config.doneRequests;
			cout<<perc<<"% <= "<<curlat<<" milliseconds"<<endl;
		}
	}
	cout<<reqpersec<<" requests per second"<<endl;
}

void clearTimeoutConnection(benchConfig* conf,uint64_t curTime)
{
	uint64_t timeout = CONNECT_TIMEOUT*1000000;
	for(unsigned i = 0; i< conf->needClientNum; i++)
	{
		client* c = conf->clients + i;
		if((c->startTime + timeout) <= curTime)
		{
			resetClient(c);
		}
	}
}

void benchmark()
{
	int epoll_fail = 0;
	//create clients we need
	createNeedClients();

	config.startTime = msTime();
	unsigned count;
	while(config.el->stop)
	{
		int num;
		num = epoll_wait(config.el->epfd,config.el->events,config.needClientNum+1,2000);
		if(num <= 0)
		{
			continue;
		}
		for(int i = 0 ;i< num; i++)
		{
			struct epoll_event *e = config.el->events + i;
			client* c = config.clients + e->data.fd%config.needClientNum;
			if(e->events & (EPOLLERR | EPOLLHUP))
			{
				if(++epoll_fail > MAX_FAIL_COUNT)
				{
					cout<<"the epoll fail count is more than "<<MAX_FAIL_COUNT<<endl;
					cout<<"the network is so poor,check it and try again."<<endl;
					exit(0);
				}
				closeConnection(&config,c->fd);
				createClient();			
				continue;
			}
			if(e->events & EPOLLIN)
			{
				c->rFuncProc(c);
			}
			if(e->events & EPOLLOUT)
			{
				c->wFuncProc(c);
			}
		}

		//check timeout connection
		if(count++ >= config.totalRequests/5)
		{
			count = 0;
			uint64_t curTime = usTime();
			clearTimeoutConnection(&config,curTime);            
		}
	}
	config.endTime = msTime();
	showReport();
}


int main(int argc,char** argv)
{
	//ignoreSignal();
	
	initConfig();
	parseOptions(argc,argv);
	config.clients = new client[config.needClientNum];
	config.latency = new uint64_t[config.totalRequests];

	config.el = createEpollLoop(config.needClientNum+1);

	benchmark();
	
	return 0;
}
