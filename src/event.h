#ifndef EVENT_H
#define EVENT_H

extern int server_sock, client_sock;
extern int epfd;

extern struct epoll_event * epoll_events;
extern struct epoll_event event;
extern struct config config;

int event_handle(SocketData * data, char * buff, int * packet_len);

#endif