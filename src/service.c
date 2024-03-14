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

//tls
void segfault_tls_handler(int signum)
{
    printf("exit dautless mqtt tls server, signal:%d\n", signum);
    exit(EXIT_SUCCESS);
}

void service_tls_start()
{
    signal(SIGINT, segfault_tls_handler);
    printf("service tls start!\n");

    SocketData * server_tls_sock = service_socket_tls_init(config.port, config.cert, config.key);
    if(server_tls_sock == NULL)
        error_exit("service socket tls init error!");

    server_socket_tls_loop(server_tls_sock);
}