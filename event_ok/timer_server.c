#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <event.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
 #include <stdlib.h>

void connection_time(int fd, short event, void *arg1)
{
	char buf[32];
	struct tm t;
	time_t now;
	struct event *arg = (struct event *)arg1;

	time(&now);
	localtime_r(&now, &t);
	asctime_r(&t, buf);

	write(fd, buf, strlen(buf));
	shutdown(fd, SHUT_RDWR);

	free(arg);
}

void connection_accept(int fd, short event, void *arg)
{
	printf("%s\n", "ha");
	/* for debugging */
	fprintf(stderr, "%s(): fd = %d, event = %d./n", __func__, fd, event);

	/* Accept a new connection. */
	struct sockaddr_in s_in;
	socklen_t len = sizeof(s_in);
	int ns = accept(fd, (struct sockaddr *) &s_in, &len);
	if (ns < 0) {
		perror("accept");
		return;
	}

	/* Install time server. */
	struct event *ev = (struct event *)malloc(sizeof(struct event));
	//event_set(&evfifo, fd, EV_WRITE , fifo_read, &evfifo);
	event_set(ev, ns, EV_WRITE, connection_time, ev);
	event_add(ev, NULL);
}

int main(void)
{
	/* Request socket. */
	int s = socket(PF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		perror("socket");
		exit(1);
	}

	/* bind() */
	struct sockaddr_in s_in;
	bzero(&s_in, sizeof(s_in));
	s_in.sin_family = AF_INET;
	s_in.sin_port = htons(7000);
	s_in.sin_addr.s_addr = INADDR_ANY;
	if (bind(s, (struct sockaddr *) &s_in, sizeof(s_in)) < 0) {
		perror("bind");
		exit(1);
	}

	/* listen() */
	if (listen(s, 5) < 0) {
		perror("listen");
		exit(1);
	}

	/* Initial libevent. */
	event_init();

	/* Create event. */
	struct event ev;
	event_set(&ev, s, EV_WRITE | EV_PERSIST, connection_accept, &ev);

	/* Add event. */
	event_add(&ev, NULL);

	event_dispatch();

	return 0;
}
