#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_event.h"
#include "../mqtt/mqtt_encode.h"
#include "../mqtt/mqtt_decode.h"
#include "../mqtt/mqtt.h"

#define DEFAULT_SIZE 128 * 128

int test_connect_packet(unsigned char * buff){
    int c_flag;
    int clean_session, will_flag, will_qos_2, will_qos_1, will_retain, user_name_flag, password_flag;
    int keep_alive;
    char client_id[23];
    char will_topic[DEFAULT_SIZE];
    char will_message[DEFAULT_SIZE];
    char user_name[DEFAULT_SIZE];
    char password[DEFAULT_SIZE];

    // fputs("select connect flag\n", stdout);

    // fputs("clean_session:", stdout);
    // scanf("%d", &clean_session);

    // fputs("will_flag:", stdout);
    // scanf("%d", &will_flag);

    // fputs("will_qos_1:", stdout);
    // scanf("%d", &will_qos_1);

    // fputs("will_qos_2:", stdout);
    // scanf("%d", &will_qos_2);

    // fputs("will_retain:", stdout);
    // scanf("%d", &will_retain);
    
    // fputs("user_name_flag:", stdout);
    // scanf("%d", &user_name_flag);
    
    // fputs("password:", stdout);
    // scanf("%d", &password_flag);
    // fgetc(stdin);

    // c_flag = password_flag * 0b10000000 + user_name_flag * 0b1000000 + will_retain * 0b100000 + will_qos_2 * 0b10000 + will_qos_1 * 0b1000 + will_flag * 0b100 + clean_session * 0b10;

    // printf("%d %d %d %d %d %d %d\n", password_flag, user_name_flag, will_retain, will_qos_2, will_qos_1, will_flag, clean_session);
    // printf("connect flag: %d\n", c_flag);

    c_flag = 2;
    keep_alive = 10;

    memcpy(buff, mqtt_conncet_encode(c_flag, keep_alive, NULL, NULL, NULL, NULL, NULL), 14);

    return *++buff + 2;
}

int test_publish_packet(unsigned char * buff){
    return mqtt_publish_encode_qos_0("hai", "hello world!", buff);
}

int test_subscribe_packet(unsigned char * buff){
    char * topics[1] = {"test"};
    int qos[1] = {0};
    int topic_size = 1;
    
    mqtt_subscribe_encode(topics, qos, topic_size, 0, buff);

    return *++buff + 2;
}