#include "config.h"
#include <signal.h>
#include "iniparser.h"
#include "log.h"

struct config config;

void config_init()
{
    memset(&config, 0, sizeof(config));
    config.port = DEFAULT_PORT;
    config.is_anonymously = DEFAULT_IS_ANONYMOUSLY;
}

void config_file_load(char *file_dir)
{
    dictionary *ini;

    ini = iniparser_load(file_dir);

    if (ini == NULL)
    {
        exit(EXIT_FAILURE);
    }

    iniparser_dump(ini, NULL);

    config.port = iniparser_getint(ini, "info:port", DEFAULT_PORT);
    config.tls = iniparser_getboolean(ini, "info:tls", 0);

    if (config.tls)
    {
        strcpy(config.key, iniparser_getstring(ini, "info:key", "NULL"));
        if (!strcmp(config.key, "NULL"))
        {
            log_error("config error! tls key\n");
            exit(EXIT_FAILURE);
        }

        strcpy(config.cert, iniparser_getstring(ini, "info:cert", "NULL"));
        if (!strcmp(config.cert, "NULL"))
        {
            log_error("config error! tls cert\n");
            exit(EXIT_FAILURE);
        }
    }

    config.log_level = iniparser_getint(ini, "log:log_level", LOG_INFO);
    config.log = iniparser_getboolean(ini, "log:log", 0);
    if(config.log)
    {
        strcpy(config.log_file, iniparser_getstring(ini, "log:log_file", "NULL"));
        if(!strcmp(config.log_file, "NULL"))
        {
            log_error("config error! log file NULL");
            exit(EXIT_FAILURE);
        }
    }

    config.is_anonymously = iniparser_getboolean(ini, "login:anonymously", DEFAULT_IS_ANONYMOUSLY);

    if (config.is_anonymously)
    {
        strcpy(config.control_type, iniparser_getstring(ini, "login:control_type", "NULL"));
        if (!strcmp(config.control_type, "NULL"))
        {
            log_error("config error! you not have set [control_type].\n");
            exit(EXIT_FAILURE);
        }

        strcpy(config.dir, iniparser_getstring(ini, "control:dir", "NULL"));
        if (!strcmp(config.dir, "NULL"))
        {
            log_error("config error! you not have set [dir].\n");
            exit(EXIT_FAILURE);
        }
    }

    iniparser_freedict(ini);
}

void log_tcp(const char *file, int line, char * info, unsigned char * buff, int len)
{
    int i, j;
    char hex[65535] = {0};

    for (i = 0, j = 0; i < len; i++)
    {        
        sprintf(&hex[j], "%02X ", *(buff++));
        j += 3;
    }

    log_log(LOG_DEBUG, file, line, "%s: %s", info, hex);
}