#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include "service.h"
#include "session.h"
#include "config.h"
#include "log.h"

extern struct config config;
FILE * log_file = NULL;

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

    config_init();  //初始化服务器配置

    while( (opt = getopt(argc, argv, "hpc:")) != -1 )
    {
        switch(opt)
        {
            case 'p':
                config.port = atoi(optarg); //服务器端口
                break;
            case 'c':
                config_file_load(optarg);   //服务器配置文件
                break;
            case '?':
                printf("格式错误\n");
            case 'h':
                printf_help();
                exit(EXIT_SUCCESS);
        }
    }
    
    if (config.is_anonymously)
    {
        if (dauntless_plugin_init(config.dir, config.control_type) < 0)
        {
            log_error("plugin init error\n");
            exit(EXIT_FAILURE);
        }
    }

    log_set_level(config.log_level);
    log_set_quiet(!config.log);

    if(config.log)
    {
        log_file = fopen(config.log_file, "ab");
        if(log_file == NULL)
        {
            log_error("can't open %s", config.log_file);
            exit(EXIT_FAILURE);
        }

        log_add_fp(log_file, LOG_INFO);
    }

    if(config.tls == 0)
        service_start();
    else
        service_tls_start();

    if(log_file) fclose(log_file);

    return 0;
}
