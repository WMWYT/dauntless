#ifndef DAUNTLESS_PLUGIN_H
#define DAUNTLESS_PLUGIN_H
#include "dauntless_mqtt.h"

typedef struct{
    char * plugin_info;
    int (* plugin_connect_handle)(struct connect_packet * connect);
    int (* plugin_publish_handle)(char * write_user_name, char * read_user_name, char * topic, char * payload);
    int (* plugin_subscribe_handle)(char * user_name, subscribe_payload * subscribe);
} dauntless_plugin_struct;

#endif