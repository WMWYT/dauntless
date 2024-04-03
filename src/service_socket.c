#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include "session.h"
#include "service.h"
#include "event.h"
#include "config.h"
#include "log.h"

int server_sock = 0;
int epfd = 0;

extern struct session *session_sock;

int service_socket_init(int server_port)
{
    struct sockaddr_in server_addr;
    socklen_t addr_size;
    int sock = 0;
    int str_len;

    server_sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(server_port);

    if(bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        log_error("bind error!");
        return -1;
    }

    if(listen(server_sock, 5) < 0)
    {
        log_error("listen error!");
        return -1;
    }

    return server_sock;
}

void server_socket_ultimately()
{
    dauntless_plugin_destroyed();
    session_delete_all();
    session_topic_delete_all();
    close(server_sock);
    close(epfd);
}

void socket_close(int fd)
{
    session_print_all();
    session_topic_print_all();
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
    log_info("close socket %d", fd);
}

int server_socket_loop(int server_sock)
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
    event.data.fd = server_sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_sock, &event);

    while(1)
    {
        memset(buff, 0, BUFF_SIZE);
        memset(recv_buffer, 0, BUFF_SIZE);
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        if(event_cnt == -1)
        {
            log_warn("epoll wait error!\n");
            return -1;
        }

        for(i = 0;i < event_cnt; i++)
        {
            if(ep_events[i].data.fd == server_sock)
            {
                addr_size = sizeof(client_addr);
                client_socket = accept(server_sock, (struct sockaddr*) &client_addr, &addr_size);
                event.events = EPOLLIN;
                event.data.fd = client_socket;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_socket, &event);
                log_info("connected client: %d",client_socket);
            }
            else
            {
                int packet_len = 0;
                str_len = read(ep_events[i].data.fd, buff, BUFF_SIZE);
                
                log_tcp_debug("tcp packet", buff, str_len);

                if(str_len > 0)
                {
                    while(str_len)
                    {
                        memmove(recv_buffer, buff + packet_len, str_len);
                        log_tcp_debug("recv buff", recv_buffer, str_len);

                        SocketData data;

                        data.fd = ep_events[i].data.fd;
                        data.ssl = NULL;
                        data.ctx = NULL;

                        int retrun_fd = event_handle(&data, recv_buffer, &packet_len);

                        if(retrun_fd < 0)
                        {
                            socket_close(ep_events[i].data.fd);
                            break;
                        }
                        else if(retrun_fd > 0)
                        {
                            socket_close(data.fd);
                        }

                        str_len -= packet_len;

                        session_print_all();
                        session_topic_print_all();
                    }
                }
                else
                {
                    struct session *s = NULL;

                    //TODO 修改好Pound节点下删除时删除不干净的问题
                    HASH_FIND(hh1, session_sock, &ep_events[i].data.fd, sizeof(int), s);
                    if (s != NULL)
                    {
                        session_close(s);
                    }
                    socket_close(ep_events[i].data.fd);
                }
            }
        }
    }
}