#include "config.h"
#include <signal.h>
#include "iniparser.h"

struct config config;

void config_init(){
    memset(&config, 0, sizeof(config));
    config.port = DEFAULT_PORT;
    config.is_anonymously = DEFAULT_IS_ANONYMOUSLY;
}

void config_file_load(char * file_dir){
    dictionary * ini;

    ini = iniparser_load(file_dir);

    if(ini == NULL){
        exit(0);
    }

    iniparser_dump(ini, stdout);

    config.port = iniparser_getint(ini, "info:port", DEFAULT_PORT);
    config.tls = iniparser_getboolean(ini, "info:tls", 0);

    config.is_anonymously = iniparser_getboolean(ini, "login:anonymously", DEFAULT_IS_ANONYMOUSLY);

    if(config.is_anonymously){
        strcpy(config.control_type, iniparser_getstring(ini, "login:control_type", NULL));
        if(config.control_type == NULL){
            printf("config error! you not have set [control_type].\n");
            exit(0);
        }

        strcpy(config.dir, iniparser_getstring(ini, "control:dir", NULL));
        if(config.dir == NULL){
            printf("config error! you not have set [dir].\n");
            exit(0);
        }
    }

    iniparser_freedict(ini);
}