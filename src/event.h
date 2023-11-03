#ifndef EVENT_H
#define EVENT_H
#include "net.h"
#include "config.h"

extern int server_sock, client_sock;
extern int epfd;

extern struct epoll_event * epoll_events;
extern struct epoll_event event;
extern struct config config;

int event_handle(int * packet_len, char * buff, int fd);

#endif