#ifndef SESSION_H
#define SESSION_H

#include "dauntless_mqtt.h"
#include "utarray.h"
#include "uthash.h"
#include "utlist.h"

struct session {
    int sock;
    char client_id[64];

    //info
    int clean_session;
    int will_qos;
    char * will_topic;
    char * will_payload;
    UT_array * topic;

    int connect_flag;

    UT_hash_handle hh1;
    UT_hash_handle hh2;
};

typedef struct{
    int qos;
    char * payload;
}publish_payload;

struct session_publish{
    char * client_id;
    char * topic;
    UT_array * payload;
};

struct session * session_add(int s_sock, char * s_client_id, int clean_session);
void session_add_will_topic(char * s_will_topic, int qos, struct session *s);
void session_add_will_payload(char * s_will_payload, struct session * s);
void session_subscribe_topic(char * s_topic, struct session *s);
void session_unsubscribe_topic(char * s_topic, struct session * s);
void session_printf_all();
void session_delete(struct session * s);
void session_delete_all();
void session_close(struct session *s);

void session_topic_subscribe(char * s_topic, int max_qos, char * s_client_id);
void session_topic_unsubscribe(char * topic, char * client_id);
void session_topic_delete_all();
UT_array * session_topic_search(char * topic);
void session_topic_printf_all();

int session_publish_add(int client_id_len, char * client_id, \
                        int qos, \
                        int topic_len, char * topic, \
                        int payload_len, char * payload);
void session_publish_delete(int packet_id);
void session_publish_printf();

#endif