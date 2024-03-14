#include "config.h"
#include <signal.h>
#include "iniparser.h"

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

    iniparser_dump(ini, stdout);

    config.port = iniparser_getint(ini, "info:port", DEFAULT_PORT);
    config.tls = iniparser_getboolean(ini, "info:tls", 0);

    if (config.tls)
    {
        strcpy(config.key, iniparser_getstring(ini, "info:key", NULL));
        if (config.key == NULL)
        {
            printf("config error! tls key\n");
            exit(EXIT_FAILURE);
        }

        strcpy(config.cert, iniparser_getstring(ini, "info:cert", NULL));
        if (config.cert == NULL)
        {
            printf("config error! tls cert\n");
            exit(EXIT_FAILURE);
        }
    }

    config.is_anonymously = iniparser_getboolean(ini, "login:anonymously", DEFAULT_IS_ANONYMOUSLY);

    if (config.is_anonymously)
    {
        strcpy(config.control_type, iniparser_getstring(ini, "login:control_type", NULL));
        if (config.control_type == NULL)
        {
            printf("config error! you not have set [control_type].\n");
            exit(EXIT_FAILURE);
        }

        strcpy(config.dir, iniparser_getstring(ini, "control:dir", NULL));
        if (config.dir == NULL)
        {
            printf("config error! you not have set [dir].\n");
            exit(EXIT_FAILURE);
        }
    }

    iniparser_freedict(ini);
}