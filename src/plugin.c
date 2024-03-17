#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include "dauntless_plugin.h"

void * handle;
dauntless_plugin_struct * dauntless_plugin;

int dauntless_plugin_destroyed(){
    if(handle)
        return dlclose(handle);
    else
        return 0;
}

int dauntless_plugin_init(char * plugin_lib_dir, char * type)
{
    char * error;
    dauntless_plugin_struct * (* dauntless_plugin_retrun)(void);

    handle = dlopen(plugin_lib_dir, RTLD_LAZY);

    if(!handle)
    {
        fprintf(stderr, "%s\n", dlerror());
        return -1;
    }

    dlerror();

    dauntless_plugin_retrun = (dauntless_plugin_struct * (*)(void))dlsym(handle, "dauntless_plugin_return");

    error = dlerror();
    if(error != NULL)
    {
        fprintf(stderr, "%s/n", error);
        return -1;
    }

    dauntless_plugin = dauntless_plugin_retrun();

    if(dauntless_plugin != NULL && strcmp(dauntless_plugin->plugin_info, type) != 0){
        printf("plugin type error.\n");
        dauntless_plugin_destroyed();
        return -1;
    }

    return 0;
}

int dauntless_plugin_connect_handle(struct connect_packet * connect)
{
    if(dauntless_plugin != NULL && dauntless_plugin->plugin_connect_handle != NULL)
        return dauntless_plugin->plugin_connect_handle(connect);

    return CONNECT_ERROR_USER_OR_PASSWORD;
}

int dauntless_plugin_subscribe_handle(char * user_name, subscribe_payload * payload)
{
    if(dauntless_plugin != NULL && dauntless_plugin->plugin_subscribe_handle != NULL)
        return dauntless_plugin->plugin_subscribe_handle(user_name, payload);

    return payload->qos;
}