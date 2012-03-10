#include <event2/event.h>
#include <event2/util.h>
#include <event2/event_struct.h>


void time_cb(int fd, short event, void* arg){
	struct timeval tv;
    struct event* timeout = (struct event*)arg;
    printf("timer ok\n");   
    tv.tv_sec=1;
    tv.tv_usec=0;

    event_add(timeout, &tv);  
}
int main(){
    struct event timeout;
    struct timeval tv;
    struct event_base* base = event_base_new();
    event_assign(&timeout, base, -1, 0, time_cb, &timeout);
    evutil_timerclear(&tv);
    
    tv.tv_sec=1;
    tv.tv_usec=0;

    event_add(&timeout, &tv);    
    event_base_dispatch(base);
    return 0;
}