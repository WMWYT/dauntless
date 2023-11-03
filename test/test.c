#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "test_event.h"
#include "../log/log.h"

#define BUF_SIZE 128

void error_handling(char * massage){
    fputs(massage, stderr);
    fputc('\n', stderr);
    exit(1);
}

int main(int argc, char * argv[]){
    int sock;
    unsigned char message[BUF_SIZE];
    unsigned char buff[BUF_SIZE];
    int str_len = 0;
    int tmp = 0;
    struct sockaddr_in serv_addr;

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1){
        error_handling("socket() error");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(1883);

    if(connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1){
        error_handling("connect() error");
    }else{
        puts("Connected........");
    }

    while(1){
        fputs("Input message(Q to quit):", stdout);
        fgets(message, BUF_SIZE, stdin);

        if(!strcmp(message, "q\n") || !strcmp(message, "Q\n")){
            break;
        }else{
            if(!strcmp(message, "connect\n")){
                tmp = test_connect_packet(buff);
                printf_buff("test connect", buff, tmp);      
                write(sock, buff, tmp);
            }
            
            if(!strcmp(message, "subscribe\n")){
                tmp = test_subscribe_packet(buff);
                printf_buff("test subscribe", buff, tmp);
                write(sock, buff, tmp);
                continue;
            }

            if(!strcmp(message, "publish\n")){
                tmp = test_publish_packet(buff);
                printf_buff("test publist", buff, tmp);
                write(sock, buff, tmp);
            }
        }

        str_len = read(sock, message, BUF_SIZE - 1);
        printf("str_len: %d\n", str_len);
        printf_buff("message", message, str_len);

        // write(sock, message, strlen(message));
        // str_len = read(sock, message, BUF_SIZE - 1);
        // message[str_len] = 0;
        // printf("Message from server: %s", message); 
    }

    close(sock);

    return 0;
}