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

        if(strcmp(connect->payload.password->string, "123") != 0 || strcmp(connect->payload.user_name->string, "WMWYT") != 0)
            return CONNECT_ERROR_USER_OR_PASSWORD;
    }

    return CONNECT_ACCEPTED;
}

dauntless_plugin_struct plugin_struct = {
    .plugin_info = "test",
    .plugin_connect_handle = plugin_connect_handle,
};

dauntless_plugin_struct * dauntless_plugin_return()
{
    return &plugin_struct;
}