#ifndef NET_H
#define NET_H
#include "event.h"

void net_start();
int control_init(const char * dl_dir, char * type);
int control_destroyed();

#endif