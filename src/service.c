#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "service.h"
#include "session.h"
#include "config.h"
#include "log.h"

int server_sock = 0, client_sock = 0;
int epfd = 0;

struct epoll_event *epoll_events;
struct epoll_event event;

extern union mqtt_packet *mqtt_packet;
extern struct session *session_sock;
extern struct session *session_client_id;
extern struct config config;

void client_close(int fd)
{
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
    printf("close socke %d\n", fd);
    // session_printf_all();
    // session_topic_printf_all();
    printf("-----------------------------------\n");
}

void close_socker()
{
    dauntless_plugin_destroyed();
    session_delete_all();
    session_topic_delete_all();
    if (mqtt_packet)
        free(mqtt_packet);
    close(server_sock);
    close(epfd);
}

void error_exit(char *error_info)
{
    fputs(error_info, stderr);
    fputc('\n', stderr);
    close_socker();
    exit(EXIT_FAILURE);
}

void segfault_handler(int signum)
{
    printf("exit dautless mqtt server, signal:%d\n", signum);
    close_socker();
    exit(EXIT_SUCCESS);
}

int server_handle(int *packet_len, char *recv_buff, int fd)
{
    int sock;
    if ((sock = event_handle(packet_len, recv_buff, fd)) < 0)
    {
        client_close(fd);
        return -1;
    }
    else if (sock > 0)
        client_close(sock);

    return 0;
}

int server_close(int fd)
{
    struct session *s = NULL;

    //TODO 修改好Pound节点下删除时删除不干净的问题
    HASH_FIND(hh1, session_sock, &fd, sizeof(int), s);
    if (s != NULL)
        session_close(s);
    client_close(fd);
}

void service_start()
{
    int server_sock = 0;

    signal(SIGINT, segfault_handler);

    if (config.is_anonymously)
        if (dauntless_plugin_init(config.dir, config.control_type) < 0)
            error_exit("control error");

    server_sock = service_socket_init(config.port);

    if (server_sock < 0)
        error_exit("server socket init");

    if (server_socket_loop(server_sock, server_handle, server_close) < 0)
        error_exit("server socket loop");
}