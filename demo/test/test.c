#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "mqtt.h"
#include "dauntless.h"

void broker_control_info(char * control_type){
    char * type = "test";
    strcpy(control_type, type);
}

int connect_call_back(void * packet){
    printf("connect_call_back\n");
    struct connect_packet * connect = (struct connect_packet *) packet;

    if(connect->payload.user_name == NULL || connect->payload.password == NULL){
        goto end;
    }

    if(!strcmp(connect->payload.user_name->string, "wmwyt") && !strcmp(connect->payload.password->string, "123")){
        return CONNECT_ACCEPTED;
    }

end:
    return CONNECT_ERROR_USER_OR_PASSWORD;
}

int subscribe_call_back(void * topic){
    printf("subscribe_call_back\n");
    char * subscribe_topic = topic;

    printf("subscribe_call_back: %s\n", subscribe_topic);

    if(strcmp(subscribe_topic, "hai") != 0)
        return -1;

    return 0;
}

int broker_control_strat(void){
    int error = 0;

    error = control_register(connect_call_back, CONNECT);
    // error = control_register(subscribe_call_back, SUBSCRIBE);

    return error;
}