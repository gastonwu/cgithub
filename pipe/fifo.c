#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <event.h>
#include <errno.h>
#include <pthread.h>

/* Lock for global stats */
static pthread_mutex_t stats_lock;

static int connectionNum = 0;
/*
 * Compile with:
 * cc -I/usr/local/include -o event-test event-test.c -L/usr/local/lib -levent
 */
static void
fifo_read(int fd, short event, void *arg);


static void
fifo_read(int fd, short event, void *arg)
{
	char buf[255];
	int len;
	struct event *ev = (struct event *)arg;

	/* Reschedule this event */
	event_add(ev, NULL);

	fprintf(stderr, "fifo_read called with fd: %d, event: %d, arg: %pn",
		fd, event, arg);
	len = read(fd, buf, sizeof(buf) - 1);

	if (len == -1) {
		perror("read");
		return;
	} else if (len == 0) {
		fprintf(stderr, "Connection closedn");
		return;
	}

	buf[len] = '\0';
	//memset(buf,'\0',strlen(buf));
	fprintf(stdout, "Read: %s\n", buf);
}

int
test ()
{
	struct event evfifo;
	struct stat st;
	const char *fifo = "event.fifo";
	int socket;

	if (lstat (fifo, &st) == 0) {
		if ((st.st_mode & S_IFMT) == S_IFREG) {
			errno = EEXIST;
			perror("lstat");
			exit (1);
		}
	}

	unlink (fifo);
	if (mkfifo (fifo, 0600) == -1) {
		perror("mkfifo");
		exit (1);
	}

	/* Linux pipes are broken, we need O_RDWR instead of O_RDONLY */
	socket = open (fifo, O_RDWR | O_NONBLOCK, 0);

	if (socket == -1) {
		perror("open");
		exit (1);
	}

	fprintf(stderr, "Write data to %s\n", fifo);
	/* Initalize the event library */
	event_init();

	/* Initalize one event */
	event_set(&evfifo, socket, EV_WRITE, fifo_read, &evfifo);

	/* Add it to the active events, without a timeout */
	event_add(&evfifo, NULL);

	event_dispatch();
	return (0);
}


void STATS_INIT() {
	pthread_mutex_init(&stats_lock, NULL);
}
void STATS_LOCK() {
    pthread_mutex_lock(&stats_lock);
}

void STATS_UNLOCK() {
    pthread_mutex_unlock(&stats_lock);
}

void test2(){

}

int main(){
	STATS_INIT();
	test();
}