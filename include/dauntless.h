#ifndef CONTROL_H
#define CONTROL_H

#include "mqtt.h"

int control_register(int (*call_back)(void *), int type);
int control_connect(struct connect_packet * connect);
int * control_subscribe(struct subscribe_packet * subscribe);

#endif