#include <stdio.h>
#include <string.h>
#include "dauntless_mqtt.h"
#include "dauntless_plugin.h"

dauntless_plugin_struct plugin_struct;

int plugin_connect_handle(struct connect_packet * connect)
{
    if(connect != NULL)
    {
        if(connect->payload.password == NULL || connect->payload.user_name == NULL)
            return CONNECT_ERROR_USER_OR_PASSWORD;

        if(strcmp(connect->payload.password->string, "123") != 0 || strcmp(connect->payload.user_name->string, "user") != 0)
            return CONNECT_ERROR_USER_OR_PASSWORD;
    }

    return CONNECT_ACCEPTED;
}

int plugin_publish_handle(char * w_user, char * r_user, char * topic, char * payload)
{
    printf("w_user -- %s\n", w_user);
    printf("r_user -- %s\n", r_user);
    printf("topic -- %s\n", topic);
    printf("payload -- %s\n", payload);

    return 0;
}

int plugin_subscribe_handle(char * user_name, subscribe_payload * subscribe)
{
    if(!strcmp(user_name, "user") && !strcmp(subscribe->topic_filter->string, "test"))
    {
        return 0X80;
    }

    return subscribe->qos;
}

dauntless_plugin_struct plugin_struct = {
    .plugin_info = "test",
    .plugin_connect_handle = plugin_connect_handle,
    .plugin_publish_handle = plugin_publish_handle,
    .plugin_subscribe_handle = plugin_subscribe_handle
};

dauntless_plugin_struct * dauntless_plugin_return()
{
    return &plugin_struct;
}