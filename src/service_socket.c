#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include "config.h"
#include "log.h"

int service_socket_init(int server_port)
{
    struct sockaddr_in server_addr;
    socklen_t addr_size;
    int server_sock;
    int sock = 0;
    int str_len;

    server_sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(server_port);

    if(bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        printf("bind error!\n");
        return -1;
    }

    if(listen(server_sock, 5) < 0)
    {
        printf("listen error!\n");
        return -1;
    }

    return server_sock;
}

int server_socket_loop(int server_socket, int (* server_handle)(int *, char *, int), int (* server_close)(int))
{
    int client_socket = 0;
    struct sockaddr_in client_addr;
    socklen_t addr_size;
    struct epoll_event * ep_events;
    struct epoll_event event;
    int epfd, event_cnt;
    int i, str_len;
    char buff[BUFF_SIZE];
    char recv_buffer[BUFF_SIZE];

    epfd = epoll_create(EPOLL_SIZE);
    ep_events = malloc(sizeof(struct epoll_event) *EPOLL_SIZE);

    event.events = EPOLLIN;
    event.data.fd = server_socket;
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_socket, &event);

    while(1)
    {
        memset(buff, 0, BUFF_SIZE);
        memset(recv_buffer, 0, BUFF_SIZE);
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        if(event_cnt == -1)
        {
            printf("epoll wait error!\n");
            return -1;
        }

        for(i = 0;i < event_cnt; i++)
        {
            if(ep_events[i].data.fd == server_socket)
            {
                addr_size = sizeof(client_addr);
                client_socket = accept(server_socket, (struct sockaddr*) &client_addr, &addr_size);
                event.events = EPOLLIN;
                event.data.fd = client_socket;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_socket, &event);
                printf("connected client: %d\n",client_socket);
            }
            else
            {
                int packet_len = 0;
                str_len = read(ep_events[i].data.fd, buff, BUFF_SIZE);
                printf("sock: %d ", ep_events[i].data.fd);
                printf_buff("read", buff, str_len);
                if(str_len > 0)
                {
                    while(str_len)
                    {
                        memmove(recv_buffer, buff + packet_len, str_len);
                        printf_buff("recv_buffer", recv_buffer, str_len);
                        
                        if(server_handle(&packet_len, recv_buffer, ep_events[i].data.fd) < 0)
                            break;
                        
                        str_len -= packet_len;
                    }
                }
                else
                {
                    server_close(ep_events[i].data.fd);
                }
            }
        }
    }
}