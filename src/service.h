#ifndef SERVICE_H
#define SERVICE_h
#include "event.h"
#include "dauntless_mqtt.h"

void service_start();

int service_socket_init(int server_port);
int server_socket_loop(int server_socket, int (* server_handle)(int *, char *, int), int (* server_close)(int));

void service_socket_tls_init();

int dauntless_plugin_init(char * plugin_lib_dir, char * type);
int dauntless_plugin_destroyed();
int dauntless_plugin_connect_handle(struct connect_packet * connect);

#endif