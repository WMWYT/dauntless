#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "config.h"
#include "service.h"
#include "log.h"

extern struct config config;
extern FILE * log_file;

void error_exit(char * error_info)
{
    log_error("%s", error_info);
    if(log_file) fclose(log_file);
    exit(EXIT_FAILURE);
}

// socket
void segfault_handler(int signum)
{
    log_info("exit dautless mqtt server, signal:%d", signum);
    server_socket_ultimately();
    if(log_file) fclose(log_file);
    exit(EXIT_SUCCESS);
}

void service_start()
{
    signal(SIGINT, segfault_handler);
    log_info("service start!");

    int server_sock = 0;

    server_sock = service_socket_init(config.port);

    if(server_sock < 0)
        error_exit("service socket init error!");

    server_socket_loop(server_sock); 
}

//tls
void segfault_tls_handler(int signum)
{
    log_info("exit dautless mqtt tls server, signal:%d", signum);
    if(log_file) fclose(log_file);
    exit(EXIT_SUCCESS);
}

void service_tls_start()
{
    signal(SIGINT, segfault_tls_handler);
    log_info("service tls start!");

    SocketData * server_tls_sock = service_socket_tls_init(config.port, config.cert, config.key);
    if(server_tls_sock == NULL)
        error_exit("service socket tls init error!");

    server_socket_tls_loop(server_tls_sock);
}