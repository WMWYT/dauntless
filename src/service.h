#ifndef SERVICE_H
#define SERVICE_h
#include <sys/socket.h>
#include <openssl/ssl.h>
#include "dauntless_mqtt.h"

#define ADDRSTRLEN 22

struct client_data
{
    char ip[ADDRSTRLEN];
    int port;
};

struct socket_data
{
    int fd;
    SSL *ssl;
    SSL_CTX *ctx;
};

typedef struct client_data ClientData;

typedef struct socket_data SocketData;

// void service_start();
// int service_socket_init(int server_port);
// int server_socket_loop(int server_socket, int (*)(int *, char *, int), int (*)(int));

// void service_tls_start();
// SocketTLS * service_socket_tls_init(int server_port, const char* public_key_fp, const char* private_key_fp);
// void service_epoll_tls_loop(SocketTLS *server_socket, int (* accept_handle)(int *, char *, int), int (* accept_close)(int));

//socket
void service_start();
int service_socket_init(int server_port);
int server_socket_loop(int server_sock);
void server_socket_ultimately();

//tls
void service_tls_start();
SocketData *service_socket_tls_init(int server_port, const char *public_key_fp, const char *private_key_fp);
void server_socket_tls_loop(SocketData *server_socket);

int dauntless_plugin_init(char *plugin_lib_dir, char *type);
int dauntless_plugin_destroyed();
int dauntless_plugin_connect_handle(struct connect_packet *connect);

#endif