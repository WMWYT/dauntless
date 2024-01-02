#ifndef NET_H
#define NET_H
#include "event.h"
#include "dauntless_mqtt.h"

void net_start();
// int control_init(const char * dl_dir, char * type);
// int control_destroyed();

int dauntless_plugin_init(char * plugin_lib_dir, char * type);
int dauntless_plugin_destroyed();
int dauntless_plugin_connect_handle(struct connect_packet * connect);

#endif