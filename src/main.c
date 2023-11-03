#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include "net.h"
#include "session.h"
#include "config.h"

extern struct config config;

void printf_help(){
    printf("dauntless [-p port] [-c file name]\n");
    printf("-p : 程序所监听端口默认为1883.\n");
    printf("-c : 配置文件所在目录\n");
}

void printf_logo(){
    printf("██████   █████  ██    ██ ███    ██ ████████ ██      ███████ ███████ ███████ \n");
    printf("██   ██ ██   ██ ██    ██ ████   ██    ██    ██      ██      ██      ██      \n");
    printf("██   ██ ███████ ██    ██ ██ ██  ██    ██    ██      █████   ███████ ███████ \n");
    printf("██   ██ ██   ██ ██    ██ ██  ██ ██    ██    ██      ██           ██      ██ \n");
    printf("██████  ██   ██  ██████  ██   ████    ██    ███████ ███████ ███████ ███████ \n");
}

int main(int argc, char * const argv[])
{
    int opt;

    printf_logo();

    config_init();

    while( (opt = getopt(argc, argv, "hpc:")) != -1 )
    {
        switch(opt)
        {
            case 'p':
                config.port = atoi(optarg);
                break;
            case 'c':
                config_file_load(optarg);
                break;
            case '?':
                printf("格式错误\n");
            case 'h':
                printf_help();
                exit(0);
                break;
        }
    }

    net_start();

    return 0;
}
