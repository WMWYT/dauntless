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
struct session_info *session_info;
struct session * session_sock = NULL;
struct session * session_client_id = NULL;
struct session_publish session_packet_identifier[65536];

char *get_rand_str(int num)
{
    int randomData = 0;
    int file = open("/dev/urandom", O_RDONLY);
    char *str = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    char *s = (char *)malloc(sizeof(char) * num + 1);
    int i, lstr;
    char ss[2] = {0};

    memset(s, 0, sizeof(char) * num + 1);
    lstr = strlen(str);

    if (file == -1)
    {
        perror("Failed to open /dev/urandom");
        exit(EXIT_FAILURE);
    }

    if (read(file, &randomData, sizeof(randomData)) == -1)
    {
        perror("Failed to read /dev/urandom");
        close(file);
        exit(EXIT_FAILURE);
    }

    close(file);

    srand(abs(randomData));
    for (i = 1; i <= num; i++)
    {
        sprintf(ss, "%c", str[(rand() % lstr)]);
        strcat(s, ss);
    }

    return s;
}

/**************session***************/
#define FREE_MQTT_STRING(A) \
    (A)->length_LSB = 0; \
    (A)->length_MSB = 0; \
    if((A)->string){ \
        free((A)->string); \
        (A)->string = NULL; \
    } \
    (A)->string_len = 0;

void free_connect_payload(connect_payload * payload)
{
    if(payload->client_id)
    {
        FREE_MQTT_STRING(payload->client_id)
    }

    if(payload->user_name)
    {
        FREE_MQTT_STRING(payload->user_name)
    }

    if(payload->password)
    {
        FREE_MQTT_STRING(payload->password)
    }

    if(payload->will_topic)
    {
        FREE_MQTT_STRING(payload->will_topic)
    }

    if(payload->will_payload)
    {
        FREE_MQTT_STRING(payload->will_payload)
    }
}

struct session *session_add(int s_sock, SSL *sock_ssl, SSL_CTX *sock_ctx, connect_payload * payload, int connect_flag) //char *s_client_id, char * user_name, int clean_session)
{
    struct session *s = NULL;
    char *client_id;

    if (payload->client_id != NULL)
    {
        HASH_FIND(hh2, session_client_id, payload->client_id, payload->client_id->string_len, s);
        if (s)
        {
            int tmp = s->sock;
            SSL * tmp_ssl = s->ssl;
            SSL_CTX * tmp_ctx = s->ctx;

            if(s->ssl != NULL)
                SSL_free(s->ssl);
            if(s->ctx != NULL)
                SSL_CTX_free(s->ctx);
            
            free_connect_payload(s->connect_info);
            utarray_free(s->topic);

            HASH_DELETE(hh1, session_sock, s);
            HASH_DELETE(hh2, session_client_id, s);

            s = (struct session *)malloc(sizeof *s);
            memset(s, 0, sizeof *s);
            s->sock = tmp;
            s->ssl = tmp_ssl;
            s->ctx = tmp_ctx;

            s->connect_info = (connect_payload * ) malloc(sizeof(connect_payload));
            memset(s->connect_info, 0, sizeof(connect_payload));
            memcpy(s->connect_info, payload, sizeof(connect_payload));

            strcpy(s->client_id, payload->client_id->string);
            utarray_new(s->topic, &ut_str_icd);
            s->clean_session = (connect_flag >> 1) & 1;
            s->will_qos = ((connect_flag >> 4) & 1) * 2 + (connect_flag >> 3) & 1;

            HASH_ADD(hh1, session_sock, sock, sizeof(int), s);
            HASH_ADD(hh2, session_client_id, client_id, strlen(s->client_id), s);

            return s;
        }
    }
    else
    {
        client_id = (char *)malloc(sizeof(char) * 9);
        strcpy(client_id, get_rand_str(8));
    }

    if (s == NULL)
    {
        s = (struct session *)malloc(sizeof *s);
        memset(s, 0, sizeof *s);
        s->sock = s_sock;
        s->ssl = sock_ssl;
        s->ctx = sock_ctx;

        s->connect_info = (connect_payload * ) malloc(sizeof(connect_payload));
        memset(s->connect_info, 0, sizeof(connect_payload));
        memcpy(s->connect_info, payload, sizeof(connect_payload));

        if(payload->client_id)
            strcpy(s->client_id, payload->client_id->string);
        else
            strcpy(s->client_id, client_id);
            
        utarray_new(s->topic, &ut_str_icd);
        s->clean_session = (connect_flag >> 1) & 1;
        s->will_qos = ((connect_flag >> 4) & 1) * 2 + (connect_flag >> 3) & 1;

        HASH_ADD(hh1, session_sock, sock, sizeof(int), s);
        HASH_ADD(hh2, session_client_id, client_id, strlen(s->client_id), s);
    }

    return s;
}

void session_delete(struct session *s)
{
    char **p = NULL;

    if (s->clean_session)
    {
        // 当删除会话的时候从session中找出topic并逐个删掉
        if (utarray_front(s->topic))
        {
            while (p = (char **)utarray_next(s->topic, p))
            {
                session_topic_unsubscribe(*p, s->client_id);
            }
        }
    }

    HASH_DELETE(hh1, session_sock, s);
    HASH_DELETE(hh2, session_client_id, s);

    if (s)
        free(s);
}

void session_subscribe_topic(char *s_topic, struct session *s)
{
    if (utarray_find(s->topic, &s_topic, strsort) == NULL)
    {
        utarray_push_back(s->topic, &s_topic);
    }

    utarray_sort(s->topic, strsort);
}

void session_unsubscribe_topic(char *s_topic, struct session *s)
{
    char **first, **find;
    long int pos = 0;

    if ((find = utarray_find(s->topic, &s_topic, strsort)) != NULL)
    {
        first = utarray_front(s->topic);
        pos = find - first;

        utarray_erase(s->topic, pos, 1);
    }

    utarray_sort(s->topic, strsort);
}

void session_printf_all()
{
    struct session *current;
    struct session *tmp;
    char **p = NULL;

    HASH_ITER(hh1, session_sock, current, tmp)
    {
        printf("sock %d: %s\n", current->sock, current->client_id);
        if(current->connect_info->user_name)
            printf(" user_name: %s\nsubscribe topic -- ", current->connect_info->user_name->string);

        if(current->connect_info->will_topic)
            printf("will topic -- %s payload -- %s\n", current->connect_info->will_topic->string, current->connect_info->will_payload->string);

        while ((p = (char **)utarray_next(current->topic, p)))
            printf("%s ", *p);
        printf("\n");
    }
}

void session_delete_all()
{
    struct session *current_1;
    struct session *current_2;
    struct session *tmp;

    HASH_ITER(hh1, session_sock, current_1, tmp)
    {
        HASH_DELETE(hh1, session_sock, current_1);
        if (current_1)
            free(current_1);
    }

    HASH_ITER(hh2, session_client_id, current_2, tmp)
    {
        HASH_DELETE(hh2, session_client_id, current_2);
        if (current_2)
            free(current_2);
    }

    current_1 = NULL;
    current_2 = NULL;
}

void publish_will_message(struct session *s)
{
    struct session *will_s;
    UT_array * will_client_id = NULL;
    int buff_size = 0;
    int qos = s->will_qos;
    char buff[65535] = {0};
    ChilderNode *p = NULL;

    if(s->connect_info->will_topic) will_client_id = session_topic_search(s->connect_info->will_topic->string);
    if (will_client_id != NULL)
    {
        if (utarray_front(will_client_id) != NULL)
        {
            while ((p = (ChilderNode *)utarray_next(will_client_id, p)))
            {
                HASH_FIND(hh2, session_client_id, p->client_id, strlen(p->client_id), will_s);

                if (s->will_qos > p->max_qos)
                    qos = p->max_qos;

                if (qos == 0)
                    buff_size = mqtt_publish_encode_qos_0(s->connect_info->will_topic->string, s->connect_info->will_payload->string, buff);
                else
                {
                    unsigned char id_M, id_L;
                    int packet_id = session_publish_add(strlen(p->client_id), p->client_id,
                                                        qos,
                                                        s->connect_info->will_topic->string_len, s->connect_info->will_topic->string,
                                                        s->connect_info->will_payload->string_len, s->connect_info->will_payload->string);
                    ML_encode(packet_id, &id_M, &id_L);

                    buff_size = mqtt_publish_encode_qos_1_2(s->connect_info->will_topic->string, qos, id_M, id_L, s->connect_info->will_payload->string, buff);
                }

                write(will_s->sock, buff, buff_size);
                memset(buff, 0, buff_size);
            }
        }
    }
}

void session_close(struct session *s)
{
    publish_will_message(s);
    session_delete(s);
}

/*******************************topic************************************/
void session_topic_subscribe(char *s_topic, int max_qos, char *s_client_id)
{
    intercept(s_topic, max_qos, s_client_id);
}

void session_topic_unsubscribe(char *topic, char *client_id)
{
    delete_topic(topic, client_id);
}

void session_topic_delete_all()
{
    if (root.childer_node != NULL)
    {
        utarray_clear(root.childer_node);
    }

    if (root.plus_children != NULL)
    {
        delete_all(root.plus_children);
    }

    if (root.children != NULL)
    {
        delete_all(root.children);
    }

    if (sys.children != NULL)
    {
        delete_all(sys.children);
    }

    if (sys.plus_children != NULL)
    {
        delete_all(sys.plus_children);
    }
}

UT_array *session_topic_search(char *topic)
{
    if (topic != NULL)
        return search(topic);

    return NULL;
}

void session_topic_printf_all()
{
    ChilderNode *p = NULL;
    printf("-------------------system-------------\n");
    printf("+\n");

    free(p);
    p = NULL;

    if (sys.plus_children != NULL)
    {
        printf_all(sys.plus_children);
    }

    printf("\nnormal\n");

    printf_all(sys.children);

    printf("-------------------root-------------\n");
    printf("#\n");
    if (root.childer_node != NULL)
    {
        while ((p = (ChilderNode *)utarray_next(root.childer_node, p)))
        {
            printf("client_id:%s ", p->client_id);
            printf("max_qos:%d ", p->max_qos);
        }
    }

    printf("\n+\n");

    free(p);
    p = NULL;

    if (root.plus_children != NULL)
    {
        printf_all(root.plus_children);
    }

    printf("\nnormal\n");

    printf_all(root.children);

    printf("____________________________________\n");
}

/*****************sessiong publish*****************/
void payload_copy(void *_dst, const void *_src)
{
    publish_payload *dst = (publish_payload *)_dst, *src = (publish_payload *)_src;
    dst->qos = src->qos;
    dst->payload = src->payload ? strdup(src->payload) : NULL;
}

void payload_dtor(void *_elt)
{
    publish_payload *elt = (publish_payload *)_elt;
    if (elt->payload != NULL)
        free(elt->payload);
    elt->qos = 0;
}

UT_icd payload_icd = {sizeof(publish_payload), NULL, payload_copy, payload_dtor};

int session_publish_add(int client_id_len, char *client_id,
                        int qos,
                        int topic_len, char *topic,
                        int payload_len, char *payload)
{
    int i = 0;
    while (i < 65535)
    {
        ++i;
        if (session_packet_identifier[i].client_id == NULL)
        {
            publish_payload *ic = (publish_payload *)malloc(sizeof(publish_payload));
            memset(ic, 0, sizeof(publish_payload));

            utarray_new(session_packet_identifier[i].payload, &payload_icd);

            ic->qos = qos;
            ic->payload = (char *)malloc(sizeof(char) * (payload_len + 1));
            memset(ic->payload, 0, sizeof(char) * (payload_len + 1));
            strcpy(ic->payload, payload);

            utarray_push_back(session_packet_identifier[i].payload, ic);

            session_packet_identifier[i].client_id = (char *)malloc(sizeof(char) + (client_id_len + 1));
            strcpy(session_packet_identifier[i].client_id, client_id);

            session_packet_identifier[i].topic = (char *)malloc(sizeof(char) + (topic_len + 1));
            strcpy(session_packet_identifier[i].topic, topic);

            break;
        }
    }

    return i;
}

void session_publish_delete(int packet_id)
{
    // TODO 在合理时间内删除会话

    char *tmp_client_id = session_packet_identifier[packet_id].client_id;
    session_packet_identifier[packet_id].client_id = NULL;
    free(tmp_client_id);

    char *tmp_topic = session_packet_identifier[packet_id].topic;
    session_packet_identifier[packet_id].topic = NULL;
    free(tmp_topic);

    utarray_free(session_packet_identifier[packet_id].payload);
}

void session_publish_printf()
{
    int i = 0;
    printf("-------------------------------------\n");
    printf("session publish\n");
    publish_payload *p;

    for (i = 0; i < 65536; i++)
    {
        if (session_packet_identifier[i].client_id != NULL)
        {
            printf("id:%d\n", i);
            printf("client_id:%s\n", session_packet_identifier[i].client_id);
            printf("topic:%s\n", session_packet_identifier[i].topic);
            p = NULL;
            while ((p = (publish_payload *)utarray_next(session_packet_identifier[i].payload, p)))
            {
                printf("payload:%s\n", p->payload);
                printf("qos:%d\n", p->qos);
            }
        }
    }
    printf("-------------------------------------\n");
}