#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include "config.h"
#include "service.h"
#include "session.h"
#include "event.h"
#include "log.h"

extern struct session *session_sock;

void socket_tls_close(SocketData **pps)
{
    if (*pps == NULL)
    {
        return;
    }
    if ((*pps)->ssl != NULL)
    {
        SSL_shutdown((*pps)->ssl); // a first time to send the close_notify alert
        SSL_shutdown((*pps)->ssl); // a second time to wait for the peer response
        SSL_free((*pps)->ssl);
    }
    if ((*pps)->ctx != NULL)
    {
        SSL_CTX_free((*pps)->ctx);
    }

    close((*pps)->fd);

    free(*pps);
    *pps = NULL;
}

SocketData *service_socket_tls_init(int server_port, const char *public_key_fp, const char *private_key_fp)
{
    SocketData *server = (SocketData *)malloc(sizeof(SocketData));
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    server->fd = service_socket_init(server_port);
    if (server->fd < 0)
    {
        socket_tls_close(&server);
        return NULL;
    }

    server->ctx = SSL_CTX_new(SSLv23_server_method());
    if (server->ctx == NULL)
    {
        socket_tls_close(&server);
        return NULL;
    }

    if (SSL_CTX_use_certificate_file(server->ctx, public_key_fp, SSL_FILETYPE_PEM) <= 0)
    {
        socket_tls_close(&server);
        return NULL;
    }

    if (SSL_CTX_use_PrivateKey_file(server->ctx, private_key_fp, SSL_FILETYPE_PEM) <= 0)
    {
        socket_tls_close(&server);
        return NULL;
    }

    return server;
}

SocketData *socket_tls_accept(SocketData* server, ClientData* pclient_data)
{
    SocketData* client;
    struct sockaddr_in peer_addr;
    unsigned int addr_size = sizeof(struct sockaddr_in);

    client = (SocketData*) malloc(sizeof(SocketData));

    if(client == NULL)
    {
        // out of memory, stop everything
        exit(1);
    }

    client->fd = accept(server->fd, (struct sockaddr*) &peer_addr, &addr_size);
    client->ssl = NULL;
    client->ctx = NULL;

    if(client->fd <= 0)
    {
        printf("%d\n", server->fd);
        socket_tls_close(&client);
        return NULL;
    }

    if(pclient_data != NULL)
    {
        inet_ntop(AF_INET, &(peer_addr.sin_addr), pclient_data->ip, INET_ADDRSTRLEN);
        pclient_data->port = ntohs(peer_addr.sin_port);
    }

    if(server->ctx != NULL)
    {
        client->ssl = SSL_new(server->ctx);
        SSL_set_fd(client->ssl, client->fd);

        if(SSL_accept(client->ssl) <= 0)
        {
            socket_tls_close(&client);
            return NULL;
        }
    }

    return client;
}

void server_socket_tls_loop(SocketData *server_socket)
{
    SocketData *client_socket;
    ClientData infos;
    struct sockaddr_in client_addr;
    socklen_t addr_size;
    struct epoll_event *ep_events;
    struct epoll_event event = {0};
    int epfd, event_cnt;
    int i, str_len;
    char buff[BUFF_SIZE];
    char recv_buffer[BUFF_SIZE];

    epfd = epoll_create(EPOLL_SIZE);
    ep_events = malloc(sizeof(struct epoll_event) * EPOLL_SIZE);

    event.events = EPOLLIN;
    event.data.ptr = (void *)server_socket;
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_socket->fd, &event);

    while (1)
    {
        memset(buff, 0, BUFF_SIZE);
        memset(recv_buffer, 0, BUFF_SIZE);
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        if (event_cnt == -1)
        {
            printf("epoll wait error!\n");
            return;
        }

        for (i = 0; i < event_cnt; i++)
        {
            SocketData *tmp = (SocketData *)ep_events[i].data.ptr;
            if (tmp->fd == server_socket->fd)
            {
                addr_size = sizeof(client_addr);
                client_socket = socket_tls_accept(server_socket, &infos);
                event.events = EPOLLIN;
                event.data.ptr = (void *)client_socket;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_socket->fd, &event);
                printf("connected client: %d\n", client_socket->fd);
            }
            else
            {
                int packet_len = 0;
                str_len = SSL_read(tmp->ssl, buff, sizeof(buff));
                printf("sock: %d ", tmp->fd);
                printf_buff("read", buff, str_len);
                if(str_len > 0)
                {
                    while(str_len)
                    {
                        memmove(recv_buffer, buff + packet_len, str_len);
                        printf_buff("recv_buffer", recv_buffer, str_len);

                        int return_fd = event_handle(tmp , recv_buffer, &packet_len);

                        if(return_fd < 0)
                        {
                            socket_tls_close(&tmp);
                            break;
                        }
                        else
                        {
                            if(return_fd > 0)
                            {
                                socket_tls_close(&tmp);
                            }
                        }
                        
                        str_len -= packet_len;
                    }
                }
                else
                {
                    struct session *s = NULL;

                    HASH_FIND(hh1, session_sock, &tmp->fd, sizeof(int), s);
                    if (s != NULL)
                        session_close(s);
                    socket_tls_close(&tmp);
                }
            }
        }
    }
}