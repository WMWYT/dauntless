#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "dauntless_mqtt.h"
#include "config.h"
#include "log.h"
#include "filtering.h"
#include "session.h"
#include "service.h"
#include "mqtt_decode.h"
#include "mqtt_encode.h"

union mqtt_packet *mqtt_packet;
extern struct config config;
extern struct session *session_sock;
extern struct session *session_client_id;
extern struct session_publish session_packet_identifier[65536];

int send_infomation(SocketData *data, char *buff, int buff_size)
{
    if (config.tls == 0)
    {
        return write(data->fd, buff, buff_size);
    }
    else
    {
        return SSL_write(data->ssl, buff, buff_size);
    }
}

int event_handle(SocketData *data, char *buff, int *packet_len)
{
    struct session *s;
    struct session_topic *st;
    struct session *session_flag;

    HASH_FIND(hh1, session_sock, &data->fd, sizeof(int), s);

    mqtt_packet = mqtt_pack_decode(buff, packet_len);

    if (mqtt_packet == NULL)
    {
        if (s)
            session_close(s);
        return -1; // 小于0 有错误，要断开链接不发送
    }

    if (mqtt_packet->connect->connect_header.control_packet_1 == CONNECT)
    {
        log_debug("packet connect");
        int error_code = mqtt_packet->connect->error_code;

        if (config.is_anonymously && error_code == CONNECT_ACCEPTED)
        {
            error_code = dauntless_plugin_connect_handle(mqtt_packet->connect);
        }

        if (error_code == CONNECT_ACCEPTED)
        {
            session_flag = session_add(data->fd, data->ssl, data->ctx, &mqtt_packet->connect->payload, mqtt_packet->connect->variable_header.connect_flags);

            send_infomation(data, mqtt_connack_encode(!(mqtt_packet->connect->variable_header.connect_flags >> 1), CONNECT_ACCEPTED),
                            4);

            if (mqtt_packet->connect->payload.client_id != NULL && !(mqtt_packet->connect->variable_header.connect_flags >> 1 & 1))
            {
                for (int i = 1; i < 65536; i++)
                {
                    if (session_packet_identifier[i].client_id != NULL)
                    {
                        if (!strcmp(session_packet_identifier[i].client_id,
                                    mqtt_packet->connect->payload.client_id->string))
                        {
                            unsigned char id_M = 0, id_L = 0;
                            int buff_size = 0;
                            char publish_buff[65535] = {0};

                            publish_payload *p_qos = NULL;
                            ML_encode(i, &id_M, &id_L);

                            // TODO 这里一次只能一个一个发
                            while (p_qos = (publish_payload *)utarray_next(session_packet_identifier[i].payload, p_qos))
                            {
                                buff_size = mqtt_publish_encode_qos_1_2(session_packet_identifier[i].topic,
                                                                        p_qos->qos,
                                                                        id_M, id_L,
                                                                        p_qos->payload,
                                                                        publish_buff);
                                send_infomation(data, publish_buff, buff_size);
                            }
                        }
                    }
                }
            }

            if (session_flag->sock != data->fd)
            {
                SocketData tmp;
                tmp.fd = session_flag->sock;
                tmp.ssl = session_flag->ssl;
                tmp.ctx = session_flag->ctx;

                session_flag->sock = data->fd;
                session_flag->ssl = data->ssl;
                session_flag->ctx = data->ctx;

                memcpy(data, &tmp, sizeof(tmp));

                return 1; // 大于0 断开之前会话client_id相同的sock
            }

            return 0;
        }
        else
        {
            send_infomation(data, mqtt_connack_encode(0, CONNECT_ERROR_AUTHORIZED), 4);
        }

        return -1;
    }

    if (s == NULL)
    {
        return -1; // 未发送connect初始化
    }

    if (mqtt_packet->publish->publish_header.control_packet_1 == PUBLISH)
    {
        log_debug("packet publish");
        struct session * publish_client;
        UT_array *publish_client_id;
        ChilderNode *p = NULL;
        int qos = mqtt_packet->publish->qos;
        int buff_size = 0;
        char publish_buff[65535] = {0};

        publish_client_id = session_topic_search(mqtt_packet->publish->variable_header.topic_name->string);

        if (publish_client_id != NULL)
        {
            while ((p = (ChilderNode *)utarray_next(publish_client_id, p)))
            {
                HASH_FIND(hh2, session_client_id, p->client_id, strlen(p->client_id), publish_client);

                if (qos > p->max_qos)
                {
                    qos = p->max_qos;
                }

                if (qos == 0)
                {
                    buff_size = mqtt_publish_encode_qos_0(mqtt_packet->publish->variable_header.topic_name->string,
                                                          mqtt_packet->publish->payload, publish_buff);
                }
                else
                {
                    unsigned char id_M, id_L;
                    int packet_id = session_publish_add(strlen(p->client_id), p->client_id,
                                                        qos,
                                                        mqtt_packet->publish->variable_header.topic_name->string_len, mqtt_packet->publish->variable_header.topic_name->string,
                                                        mqtt_packet->publish->payload_len, mqtt_packet->publish->payload);
                    ML_encode(packet_id, &id_M, &id_L);

                    buff_size = mqtt_publish_encode_qos_1_2(mqtt_packet->publish->variable_header.topic_name->string, qos, id_M, id_L, mqtt_packet->publish->payload, publish_buff);
                }

                if (publish_client != NULL)
                {
                    SocketData tmp;
                    tmp.fd = publish_client->sock;
                    tmp.ssl = publish_client->ssl;
                    tmp.ctx = publish_client->ctx;

                    if(config.is_anonymously)
                    {
                        dauntless_plugin_publish_handle(s->connect_info->user_name->string, 
                                                        publish_client->connect_info->user_name->string, 
                                                        mqtt_packet->publish->variable_header.topic_name->string,
                                                        mqtt_packet->publish->payload);
                    }
                    
                    send_infomation(&tmp, publish_buff, buff_size);
                }
            }
        }

        if (mqtt_packet->publish->qos == 1)
        {
            send_infomation(data, mqtt_publish_qos_encode(PUBACK, 0, mqtt_packet->publish->variable_header.identifier_MSB, mqtt_packet->publish->variable_header.identifier_LSB), 4);
        }

        if (mqtt_packet->publish->qos == 2)
        {
            send_infomation(data, mqtt_publish_qos_encode(PUBREC, 0, mqtt_packet->publish->variable_header.identifier_MSB, mqtt_packet->publish->variable_header.identifier_LSB), 4);
        }
    }

    // QOS1
    if (mqtt_packet->const_packet->const_header.control_packet_1 == PUBACK)
    {
        log_debug("puback packet");
        unsigned char buff[2] = {0};
        buff[0] = mqtt_packet->const_packet->variable_header.byte1;
        buff[1] = mqtt_packet->const_packet->variable_header.byte2;
        session_publish_delete(string_len(buff));
    }

    // QOS2
    if (mqtt_packet->const_packet->const_header.control_packet_1 == PUBREL)
    {
        log_debug("pubrel packet");
        session_publish_print();
        send_infomation(data, mqtt_publish_qos_encode(PUBCOMP, 0, mqtt_packet->const_packet->variable_header.byte1, mqtt_packet->const_packet->variable_header.byte2), 4);
    }

    if (mqtt_packet->const_packet->const_header.control_packet_1 == PUBREC)
    {
        log_debug("pubrec packet");
        unsigned char buff[2] = {0};
        buff[0] = mqtt_packet->const_packet->variable_header.byte1;
        buff[1] = mqtt_packet->const_packet->variable_header.byte2;
        session_publish_delete(string_len(buff));

        send_infomation(data, mqtt_publish_qos_encode(PUBREL, 2, mqtt_packet->const_packet->variable_header.byte1, mqtt_packet->const_packet->variable_header.byte2), 4);
    }

    if (mqtt_packet->const_packet->const_header.control_packet_1 == PUBCOMP)
    {
        log_debug("pubcomp packet");
        // TODO 丢弃状态
    }

    if (mqtt_packet->subscribe->subscribe_header.control_packet_1 == SUBSCRIBE)
    {
        log_debug("subscribe packet");
        int *return_code = (int *)malloc(sizeof(int) * (mqtt_packet->subscribe->topic_size + 1));
        memset(return_code, 0, sizeof(return_code));

        for (int i = 0; i < mqtt_packet->subscribe->topic_size; i++)
        {
            if(config.is_anonymously)
            {
                return_code[i] = dauntless_plugin_subscribe_handle(s->connect_info->user_name->string, &mqtt_packet->subscribe->payload[i]);
            }
            else
            {
                return_code[i] = mqtt_packet->subscribe->payload->qos;
            }

            if (return_code == NULL || return_code[i] != 0x80)
            {
                session_subscribe_topic(mqtt_packet->subscribe->payload[i].topic_filter->string, s);
                session_topic_subscribe(mqtt_packet->subscribe->payload[i].topic_filter->string,
                                        mqtt_packet->subscribe->payload[i].qos,
                                        s->client_id);
            }
        }

        send_infomation(data, mqtt_suback_encode(mqtt_packet->subscribe->variable_header.identifier_MSB, 
                                                 mqtt_packet->subscribe->variable_header.identifier_LSB, 
                                                 mqtt_packet->subscribe->topic_size, return_code),
                        mqtt_packet->subscribe->topic_size + 4);

        if(return_code) free(return_code);
    }

    if (mqtt_packet->unsubscribe->unsubscribe_header.control_packet_1 == UNSUBSCRIBE)
    {
        log_debug("unsubscribe packet");
        for (int i = 0; i < mqtt_packet->unsubscribe->un_topic_size; i++)
        {
            session_unsubscribe_topic(mqtt_packet->unsubscribe->payload[i]->string, s);
            session_topic_unsubscribe(mqtt_packet->unsubscribe->payload[i]->string, s->client_id);
        }

        send_infomation(data, mqtt_unsuback_encode(mqtt_packet->unsubscribe->variable_header.identifier_MSB, mqtt_packet->unsubscribe->variable_header.identifier_LSB), 4);
    }

    if (mqtt_packet->pingreq->pingreq_header.control_packet_1 == PINGREQ)
    {
        log_debug("pingreq packet");
        send_infomation(data, mqtt_pingresp_encode(), 2);
    }

    if (mqtt_packet->disconnect->disconnect_header.control_packet_1 == DISCONNECT)
    {
        log_debug("disconnect packet");
        session_close(s);

        return -1; // 小于0 客户端断开链接， 要断开链接不发送
    }

    free(mqtt_packet);

    return 0; // 等于0 没问题
}