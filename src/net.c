#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "net.h"
#include "session.h"
#include "dauntless.h"
#include "config.h"
#include "log.h"

int server_sock, client_sock;
int epfd;

struct epoll_event * epoll_events;
struct epoll_event event;

extern union mqtt_packet * mqtt_packet;
extern struct session * session_sock;
extern struct session * session_client_id;
extern struct config config;

void client_close(int fd){
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
    printf("close socke %d\n", fd);
    // session_printf_all();
    // session_topic_printf_all();
    printf("-----------------------------------\n");
}

void close_socker(){
    control_destroyed();
    session_info_delete();
    session_delete_all();
    session_topic_delete_all();
    if(mqtt_packet) free(mqtt_packet);
    close(server_sock);
    close(epfd);
}

void error_exit(char * error_info){
    fputs(error_info, stderr);
    fputc('\n', stderr);
    exit(1);
}

void segfault_handler(int signum) {
    printf("Segmentation fault (SIGSEGV) caught %d.\n", signum);
    close_socker();
    exit(EXIT_FAILURE);
}

void net_start(){
    signal(SIGSEGV, segfault_handler);

    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    int sock = 0;
    int str_len, i;
    signed char buff[BUFF_SIZE];
    char recv_buffer[BUFF_SIZE];

    int event_cnt;

    server_sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("config port:%d\n", config.port);
    server_addr.sin_port = htons(config.port);

    if(config.is_anonymously)
        if(control_init(config.dir, config.control_type) == -1){
            error_exit("control error");
        }

    if(bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
        error_exit("bind error");
    }

    if(listen(server_sock, 5) == -1){
        error_exit("listen error");
    }

    epfd = epoll_create(EPOLL_SIZE);
    epoll_events = malloc(sizeof(struct epoll_event) * EPOLL_SIZE);

    event.events = EPOLLIN;
    event.data.fd = server_sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_sock, &event);

    while(1){
        memset(buff, 0, BUFF_SIZE);
        memset(recv_buffer, 0, BUFF_SIZE);
        event_cnt = epoll_wait(epfd, epoll_events, EPOLL_SIZE, -1);
        if(event_cnt == -1){
            error_exit("epoll_wait error");
        }

        for(i = 0; i < event_cnt; i++){
            if(epoll_events[i].data.fd == server_sock){
                addr_size = sizeof(client_addr);
                client_sock = accept(server_sock, (struct sockaddr *) &client_addr, &addr_size);
                event.events = EPOLLIN;
                event.data.fd = client_sock;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_sock, &event);
                printf("connected client: %d\n", client_sock);
            }else{
                //TODO 响应在合理时间内未受到connect包应该断开链接
                str_len = read(epoll_events[i].data.fd, buff, BUFF_SIZE);
                printf("sock: %d ", epoll_events[i].data.fd);
                printf_buff("read", buff, str_len);
                int packet_len = 0;
                if(str_len > 0){
                    while(str_len){
                        memmove(recv_buffer, buff + packet_len, str_len);
                        printf_buff("recv_buffer", recv_buffer, str_len);
                        if((sock = event_handle(&packet_len, recv_buffer, epoll_events[i].data.fd)) < 0){
                            client_close(epoll_events[i].data.fd);
                            break;
                        }else if(sock > 0){
                            client_close(sock);
                        }

                        str_len -= packet_len;
                    }

                    session_publish_printf();
                    // session_printf_all();
                    // session_topic_printf_all();
                }else{
                    struct session * s = NULL;
 
                    HASH_FIND(hh1, session_sock, &epoll_events[i].data.fd, sizeof(int), s);
                    if(s != NULL) session_close(s);
                    client_close(epoll_events[i].data.fd);
                }
            }
        }
    }

    close_socker();
}