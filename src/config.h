#ifndef CONFIG_H
#define CONFIG_H

#define DEFAULT_PORT 1883
#define DEFAULT_IS_ANONYMOUSLY 0
#define BUFF_SIZE 128
#define EPOLL_SIZE 128

#define log_tcp_debug(info, buff, len) log_tcp(__FILE__, __LINE__, info, buff, len)

struct config{
    // info
    int port;
    int tls;
    char key[256];
    char cert[256];

    // log
    int log;
    int log_level;
    char log_file[256];

    // plug_in
    int is_anonymously;
    char control_type[64];
    char dir[256];
};

void log_tcp(const char *file, int line, char * info, unsigned char * buff, int len);
void config_init();
void config_file_load(char * file_dir);

#endif