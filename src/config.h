#ifndef CONFIG_H
#define CONFIG_H

#define DEFAULT_PORT 1883
#define DEFAULT_IS_ANONYMOUSLY 0
#define BUFF_SIZE 128
#define EPOLL_SIZE 128

struct config{
    int port;
    int is_anonymously;
    char control_type[64];
    char dir[256];
};

void config_init();
void config_file_load(char * file_dir);

#endif