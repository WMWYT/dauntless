#ifndef DAUNTLESS_PLUGIN_H
#define DAUNTLESS_PLUGIN_H
#include "dauntless_mqtt.h"

typedef struct{
    char * plugin_info;
    int (* plugin_connect_handle)(struct connect_packet * connect);
} dauntless_plugin_struct;

#endif