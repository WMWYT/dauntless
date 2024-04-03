#ifndef SESSION_H
#define SESSION_H

#include <openssl/ssl.h>
#include "dauntless_mqtt.h"
#include "utarray.h"
#include "uthash.h"
#include "utlist.h"

struct session {
    // key
    int sock;
    char client_id[64];

    // tls
    SSL *ssl;
    SSL_CTX *ctx;

    //info
    int will_qos;
    int clean_session;
    connect_payload * connect_info;

    UT_array * topic;

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

// struct session * session_add(int s_sock, SSL *sock_ssl, SSL_CTX *sock_ctx, char *s_client_id, char * user_name, int clean_session);
struct session *session_add(int s_sock, SSL *sock_ssl, SSL_CTX *sock_ctx, connect_payload * payload, int clean_session);
void session_subscribe_topic(char * s_topic, struct session *s);
void session_unsubscribe_topic(char * s_topic, struct session * s);
void session_print_all();
void session_delete(struct session * s);
void session_delete_all();
void session_close(struct session *s);

void session_topic_subscribe(char * s_topic, int max_qos, char * s_client_id);
void session_topic_unsubscribe(char * topic, char * client_id);
void session_topic_delete_all();
UT_array * session_topic_search(char * topic);
void session_topic_print_all();

int session_publish_add(int client_id_len, char * client_id, \
                        int qos, \
                        int topic_len, char * topic, \
                        int payload_len, char * payload);
void session_publish_delete(int packet_id);
void session_publish_print();

#endif