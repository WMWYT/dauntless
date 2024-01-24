#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "session.h"
#include "filtering.h"
#include "mqtt_encode.h"

extern struct RootNode root;
extern struct SYSNode sys;
struct session_info * session_info;
struct session *session_sock;
struct session *session_client_id;
struct session_publish session_packet_identifier[65536];

char * get_rand_str(int num){
    int randomData = 0;
    int file = open("/dev/urandom", O_RDONLY);
    char *str = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    char * s = (char *) malloc(sizeof(char) * num + 1);
    int i,lstr;
    char ss[2] = {0};

    memset(s, 0, sizeof(char) * num + 1);
    lstr = strlen(str);

    if (file == -1) {
        perror("Failed to open /dev/urandom");
        exit(EXIT_FAILURE);
    }
    
    if (read(file, &randomData, sizeof(randomData)) == -1) {
        perror("Failed to read /dev/urandom");
        close(file);
        exit(EXIT_FAILURE);
    }

    close(file);

    srand(abs(randomData));
    for(i = 1; i <= num; i++){
        sprintf(ss,"%c",str[(rand() % lstr)]);
        strcat(s,ss);
    }

    return s;
}

/**************session***************/
struct session * session_add(int s_sock, char * s_client_id, int clean_session){
    struct session * s = NULL;
    char * client_id;
    int tmp = 0;

    if(s_client_id != NULL){
        HASH_FIND(hh2, session_client_id, s_client_id, strlen(s_client_id), s);
        if (s){
            tmp = s->sock;
            utarray_free(s->topic);
            if(s->will_topic) free(s->will_topic);
            if(s->will_topic) free(s->will_payload);
            HASH_DELETE(hh1, session_sock, s);
            HASH_DELETE(hh2, session_client_id, s);

            s = (struct session *) malloc(sizeof * s);
            memset(s, 0, sizeof *s);
            s->sock = tmp;
            strcpy(s->client_id, s_client_id);
            utarray_new(s->topic, &ut_str_icd);
            s->clean_session = clean_session;
            s->connect_flag = 1;

            HASH_ADD(hh1, session_sock, sock, sizeof(int), s);
            HASH_ADD(hh2, session_client_id, client_id, strlen(s->client_id), s);

            return s;
        }
    } else {
        s_client_id = (char *) malloc(sizeof(char) * 9);
        strcpy(s_client_id, get_rand_str(8));
    }

    if(s == NULL){
        s = (struct session *) malloc(sizeof *s);
        memset(s, 0, sizeof *s);
        s->sock = s_sock;
        strcpy(s->client_id, s_client_id);
        utarray_new(s->topic, &ut_str_icd);
        s->clean_session = clean_session;

        HASH_ADD(hh1, session_sock, sock, sizeof(int), s);
        HASH_ADD(hh2, session_client_id, client_id, strlen(s->client_id), s);
    }

    return s;
}

void session_delete(struct session * s){
    char **p = NULL;

    if(s->clean_session){
        //当删除会话的时候从session中找出topic并逐个删掉
        if(utarray_front(s->topic)){
            while(p = (char **)utarray_next(s->topic, p)){
                session_topic_unsubscribe(*p, s->client_id);
            }
        }
    }

    HASH_DELETE(hh1, session_sock, s);
    HASH_DELETE(hh2, session_client_id, s);

    if(s)
        free(s);
}

void session_add_will_topic(char * s_will_topic, int qos, struct session *s){
    if(s->will_topic == NULL){
        s->will_topic = (char *) malloc(sizeof(char) * (strlen(s_will_topic) + 1));
        memset(s->will_topic, 0, sizeof(char) * (strlen(s_will_topic) + 1));
    }

    s->will_qos = qos;
    printf("session_add_will_topic:%d\n", s->will_qos);
    memmove(s->will_topic, s_will_topic, strlen(s_will_topic));
}

void session_add_will_payload(char * s_will_payload, struct session * s){
    if(s->will_payload == NULL){
        s->will_payload = (char *)malloc(sizeof(char) * (strlen(s_will_payload) + 1));
        memset(s->will_payload, 0, sizeof(char) * (strlen(s_will_payload) + 1));
    }

    memmove(s->will_payload, s_will_payload, strlen(s_will_payload));
}

void session_subscribe_topic(char * s_topic, struct session *s){
    if(utarray_find(s->topic, &s_topic, strsort) == NULL){
        utarray_push_back(s->topic, &s_topic);
    }
    
    utarray_sort(s->topic, strsort);
}

void session_unsubscribe_topic(char * s_topic, struct session * s){
    char **first, **find;
    long int pos = 0;
    
    if((find = utarray_find(s->topic, &s_topic, strsort)) != NULL){
        first = utarray_front(s->topic);
        pos = find - first;

        utarray_erase(s->topic, pos, 1);
    }

    utarray_sort(s->topic, strsort);
}

void session_printf_all(){
    struct session *current;
    struct session *tmp;
    char **p = NULL;

    HASH_ITER(hh1, session_sock, current, tmp) {
        printf("sock %d: %s subscribe topic -- ", current->sock, current->client_id);
        while((p = (char **) utarray_next(current->topic, p)))
           printf("%s ", *p);
        printf("\n");

        printf("will topic -- %s payload -- %s\n", current->will_topic, current->will_payload);
    }
}

void session_delete_all(){
    struct session *current_1;
    struct session *current_2;
    struct session *tmp;

    HASH_ITER(hh1, session_sock, current_1, tmp) {
        HASH_DELETE(hh1, session_sock, current_1);
        if(current_1)
            free(current_1);
    }

    HASH_ITER(hh2, session_client_id, current_2, tmp){
        HASH_DELETE(hh2, session_client_id, current_2);
        if(current_2)
            free(current_2);
    }

    current_1 = NULL;
    current_2 = NULL;
}

void publish_will_message(struct session * s){
    struct session * will_s;
    UT_array * will_client_id;
    int buff_size = 0;
    int qos = s->will_qos;
    char buff[65535] = {0};
    ChilderNode * p = NULL;

    will_client_id = session_topic_search(s->will_topic);
    if(will_client_id != NULL){
        if(utarray_front(will_client_id) != NULL){
            while((p = (ChilderNode *) utarray_next(will_client_id, p))){
                HASH_FIND(hh2, session_client_id, p->client_id, strlen(p->client_id), will_s);
                
                if(s->will_qos > p->max_qos)
                    qos = p->max_qos;

                if(qos == 0)
                    buff_size = mqtt_publish_encode_qos_0(s->will_topic, s->will_payload, buff);
                else{
                    unsigned char id_M, id_L;
                    int packet_id = session_publish_add(strlen(p->client_id), p->client_id, \
                                        qos, \
                                        strlen(s->will_topic), s->will_topic, \
                                        strlen(s->will_payload), s->will_payload);
                    ML_encode(packet_id, &id_M, &id_L);

                    buff_size = mqtt_publish_encode_qos_1_2(s->will_topic, qos, id_M, id_L, s->will_payload, buff);
                }

                write(will_s->sock, buff, buff_size);
                memset(buff, 0, buff_size);
            }
        }
    }
}

void session_close(struct session *s){
    publish_will_message(s);
    session_delete(s);
}

/*******************************topic************************************/
void session_topic_subscribe(char * s_topic, int max_qos, char * s_client_id){
    intercept(s_topic, max_qos, s_client_id);
}

void session_topic_unsubscribe(char * topic, char * client_id){
    delete_topic(topic, client_id);
}

void session_topic_delete_all(){
    if(root.childer_node != NULL){
        utarray_clear(root.childer_node);
    }

    if(root.plus_children != NULL){
        delete_all(root.plus_children);
    }

    if(root.children != NULL){
        delete_all(root.children);
    }

    if(sys.children != NULL){
        delete_all(sys.children);
    }

    if(sys.plus_children != NULL){
        delete_all(sys.plus_children);
    }
}

UT_array * session_topic_search(char * topic){
    if(topic != NULL)
        return search(topic);
    
    return NULL;
}

void session_topic_printf_all(){
    ChilderNode *p = NULL;
    printf("-------------------system-------------\n");
    printf("+\n");

    free(p);
    p = NULL;

    if(sys.plus_children != NULL){
        printf_all(sys.plus_children);
    }

    printf("\nnormal\n");

    printf_all(sys.children);


    printf("-------------------root-------------\n");
    printf("#\n");
    if(root.childer_node != NULL){
        while ( (p=(ChilderNode *)utarray_next(root.childer_node,p))) {
            printf("client_id:%s ", p->client_id);
            printf("max_qos:%d ", p->max_qos);
        }
    }

    printf("\n+\n");

    free(p);
    p = NULL;

    if(root.plus_children != NULL){
        printf_all(root.plus_children);
    }

    printf("\nnormal\n");

    printf_all(root.children);

    printf("____________________________________\n");
}

/*****************sessiong publish*****************/
void payload_copy(void *_dst, const void *_src) {
  publish_payload *dst = (publish_payload*)_dst, *src = (publish_payload*)_src;
  dst->qos = src->qos;
  dst->payload = src->payload ? strdup(src->payload) : NULL;
}

void payload_dtor(void *_elt) {
    publish_payload *elt = (publish_payload*)_elt;
    if (elt->payload != NULL) free(elt->payload);
    elt->qos = 0;
}

UT_icd payload_icd = {sizeof(publish_payload), NULL, payload_copy, payload_dtor};

int session_publish_add(int client_id_len, char * client_id, \
                        int qos, \
                        int topic_len, char * topic, \
                        int payload_len, char * payload){
    int i = 0;
    while(i < 65535){
        ++i;
        if(session_packet_identifier[i].client_id == NULL){
            publish_payload * ic = (publish_payload *) malloc(sizeof(publish_payload));
            memset(ic, 0, sizeof(publish_payload));

            utarray_new(session_packet_identifier[i].payload, &payload_icd);

            ic->qos = qos;
            ic->payload = (char *) malloc(sizeof(char) * (payload_len + 1));
            memset(ic->payload, 0, sizeof(char) * (payload_len + 1));
            strcpy(ic->payload, payload);

            utarray_push_back(session_packet_identifier[i].payload, ic);

            session_packet_identifier[i].client_id = (char *) malloc(sizeof(char) + (client_id_len + 1));
            strcpy(session_packet_identifier[i].client_id, client_id);

            session_packet_identifier[i].topic = (char *) malloc(sizeof(char) + (topic_len + 1));
            strcpy(session_packet_identifier[i].topic, topic);

            break;
        }
    }

    return i;
}

void session_publish_delete(int packet_id){
    //TODO 在合理时间内删除会话

    char * tmp_client_id = session_packet_identifier[packet_id].client_id;
    session_packet_identifier[packet_id].client_id = NULL;
    free(tmp_client_id);

    char * tmp_topic = session_packet_identifier[packet_id].topic;
    session_packet_identifier[packet_id].topic = NULL;
    free(tmp_topic);

    utarray_free(session_packet_identifier[packet_id].payload);
}

void session_publish_printf(){
    int i = 0;
    printf("-------------------------------------\n");
    printf("session publish\n");
    publish_payload *p;
    
    for(i = 0; i < 65536; i++){
        if(session_packet_identifier[i].client_id != NULL){
            printf("id:%d\n", i);
            printf("client_id:%s\n", session_packet_identifier[i].client_id);
            printf("topic:%s\n", session_packet_identifier[i].topic);
            p = NULL;
            while((p = (publish_payload *) utarray_next(session_packet_identifier[i].payload, p))){
                printf("payload:%s\n", p->payload);
                printf("qos:%d\n", p->qos);
            }
        }
    }
    printf("-------------------------------------\n");
}