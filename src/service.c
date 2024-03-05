// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <signal.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <arpa/inet.h>
// #include <sys/epoll.h>
// #include "service.h"
// #include "session.h"
// #include "config.h"
// #include "log.h"

// int server_sock = 0;
// int epfd = 0;

// SocketTLS * server_tls_sock;

// extern union mqtt_packet *mqtt_packet;
// extern struct session *session_sock;
// extern struct config config;

// //socket
// void client_close(int fd)
// {
//     epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
//     close(fd);
//     printf("close socke %d\n", fd);
//     // session_printf_all();
//     // session_topic_printf_all();
//     printf("-----------------------------------\n");
// }

// void socket_close()
// {
//     dauntless_plugin_destroyed();
//     session_delete_all();
//     session_topic_delete_all();
//     if (mqtt_packet)
//         free(mqtt_packet);
//     close(server_sock);
//     close(epfd);
// }

// void error_exit(char *error_info)
// {
//     fputs(error_info, stderr);
//     fputc('\n', stderr);
//     exit(EXIT_FAILURE);
// }

// void segfault_handler(int signum)
// {
//     printf("exit dautless mqtt server, signal:%d\n", signum);
//     //socket_close();
//     exit(EXIT_SUCCESS);
// }

// int accept_handle(int *packet_len, char *recv_buff, int fd)
// {
//     int sock;
//     if ((sock = event_handle(packet_len, recv_buff, fd)) < 0)
//     {
//         client_close(fd);
//         return -1;
//     }
//     else if (sock > 0)
//         client_close(sock);

//     return 0;
// }

// int accept_close(int fd)
// {
//     struct session *s = NULL;

//     //TODO 修改好Pound节点下删除时删除不干净的问题
//     HASH_FIND(hh1, session_sock, &fd, sizeof(int), s);
//     if (s != NULL)
//         session_close(s);
//     client_close(fd);
// }

// void service_start()
// {
//     int server_sock = 0;

//     signal(SIGINT, segfault_handler);

//     server_sock = service_socket_init(config.port);

//     if (server_sock < 0){
//         //socket_close();
//         error_exit("server socket init");
//     }

//     if (server_socket_loop(server_sock, accept_handle, accept_close) < 0){
//         //socket_close();
//         error_exit("server socket loop");
//     }
// }

// //tls
// void service_tls_start()
// {
//     signal(SIGINT, segfault_handler);

//     server_tls_sock = service_socket_tls_init(config.port, "cert.pem", "key.pem");
//     if(server_tls_sock == NULL)
//         error_exit("server socket init error!");

//     service_epoll_tls_loop(server_tls_sock, accept_handle, accept_close);
// }

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "config.h"
#include "service.h"

extern struct config config;

void error_exit(char * error_info)
{
    printf("%s\n", error_info);
    exit(EXIT_FAILURE);
}

void segfault_handler(int signum)
{
    printf("exit dautless mqtt server, signal:%d\n", signum);
    server_socket_ultimately();
    exit(EXIT_SUCCESS);
}

void service_start()
{
    signal(SIGINT, segfault_handler);
    printf("service start!\n");

    int server_sock = 0;

    server_sock = service_socket_init(config.port);

    if(server_sock < 0)
        error_exit("service socket init error!");

    server_socket_loop(server_sock); 
}

void segfault_tls_handler(int signum)
{
    printf("exit dautless mqtt tls server, signal:%d\n", signum);
    exit(EXIT_SUCCESS);
}

void service_tls_start()
{
    signal(SIGINT, segfault_tls_handler);
    printf("service tls start!\n");
}