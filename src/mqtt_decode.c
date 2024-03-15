#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mqtt_decode.h"
#include "dauntless_mqtt.h"
#include "log.h"

int string_len(unsigned char * buff){
    int i = 1;
    int multiplier = 1, value = 0;
    
    do{
        value += (buff[i] & 0x7F) * multiplier;

        multiplier *= 0x80;
        if(value > 128 * 128){
            //TODO 当字符串长度超出长度时跳出并断开链接
            value = 0;
            fprintf(stderr, "protocol name len error\n");
            break;
        }
    }while( buff[i--] & 0x80 );

    return value;
}

struct mqtt_string * hex_to_string(unsigned char * buff){
    int value = string_len(buff);

    struct mqtt_string * string = (struct mqtt_string *) malloc(sizeof(struct mqtt_string));
    memset(string, 0, sizeof(struct mqtt_string));

    string->length_MSB = *buff++;
    string->length_LSB = *buff++;
    
    string->string_len = value;

    if(value != 0){
        string->string = (unsigned char *) malloc(sizeof(unsigned char) * (value + 1));
        memset(string->string, 0, value + 1);

        //TODO 有U+0000和U+D800到U+DFFF的字符时关闭链接
        strncpy(string->string, buff, value);
        if(strlen(string->string) != value){        
            return NULL;
        }
    }else{
        string->string = NULL;
    }

    return string;
}

struct connect_packet *mqtt_connect_packet_create(struct fixed_header header, unsigned char * buff){
    if(header.control_packet_1 != 1 || header.control_packet_2 != 0){
        return NULL;
    }

    struct connect_packet * packet = (struct connect_packet *) malloc(sizeof(struct connect_packet));
    packet->error_code = CONNECT_ACCEPTED;
    memset(packet, 0, sizeof(struct connect_packet));
    packet->connect_header = header;

    packet->variable_header.protocol_name = hex_to_string(buff);

    if(strcmp(packet->variable_header.protocol_name->string, "MQTT")){
        printf("error not mqtt packet\n");
        return NULL;
    }

    buff += packet->variable_header.protocol_name->string_len + 2;

    packet->variable_header.protocol_level = *buff++;

    if(packet->variable_header.protocol_level != 0x04){
        fprintf(stderr, "error procol level\n");
        packet->error_code = CONNECT_ERROR_VERSION;
        return packet;
    }

    packet->variable_header.connect_flags = *buff++;

    if(packet->variable_header.connect_flags & 0){
        fprintf(stderr, "error connect flag reserved\n");
        return NULL;
    }

    packet->variable_header.keep_alive_MSB = *buff++;
    packet->variable_header.keep_alive_LSB = *buff++;

    packet->payload.client_id = hex_to_string(buff);
    buff += packet->payload.client_id->string_len + 2;

    if(packet->payload.client_id->string_len >= 64){
        return NULL;
    }

    if(packet->payload.client_id->string_len == 0 && packet->variable_header.connect_flags >> 1 & 0){
        fprintf(stderr, "error, client id and connect flag not match\n");
        packet->error_code = CONNECT_ERROR_IDENTIFIER;
        return packet;
    }

    if((packet->variable_header.connect_flags >> 2) & 1){
        packet->payload.will_topic = hex_to_string(buff);
        buff += packet->payload.will_topic->string_len + 2;
        
        packet->payload.will_payload = hex_to_string(buff);
        buff += packet->payload.will_payload->string_len + 2;
    }

    if((packet->variable_header.connect_flags >> 7) & 1){
        packet->payload.user_name = hex_to_string(buff);
        buff += packet->payload.user_name->string_len + 2;
    }

    if((packet->variable_header.connect_flags >> 6) & 1){
        packet->payload.password = hex_to_string(buff);
        buff += packet->payload.password->string_len + 2;
    }
    
    return packet;
}

int topic_buff_len(unsigned char * buff){
    int i;

    for (i = 0; (*buff || *(buff + 1)) && i < 128; i++) {
        hex_to_string(buff);
    }
}

int subscribe_topic_len(unsigned char * buff){
    int i = 0, j = 0, value = 0;

    for(i = 0; *buff || *(buff + 1); i++){
        j = string_len(buff);
        buff += j + 3;
        value++;
    }

    return value;
}

struct subscribe_packet * mqtt_subscribe_packet_create(struct fixed_header header, unsigned char * buff){
    if(header.control_packet_1 != 8 || header.control_packet_2 != 2){
        return NULL;
    }

    struct subscribe_packet * packet = (struct subscribe_packet *) malloc(sizeof(struct subscribe_packet));
    memset(packet, 0, sizeof(struct subscribe_packet));
    packet->subscribe_header = header;
    int i, topics;

    packet->variable_header.identifier_MSB = *buff++;
    packet->variable_header.identifier_LSB = *buff++;

    topics = subscribe_topic_len(buff);

    packet->payload = (struct subscribe_payload *) malloc(sizeof(struct subscribe_payload) * topics);
    memset(packet->payload, 0, sizeof(struct subscribe_payload) * topics);

    for(i = 0; *buff || *(buff + 1); i++){
        packet->payload[i].topic_filter = hex_to_string(buff);
        buff += packet->payload[i].topic_filter->string_len + 2;
        packet->payload[i].qos = *buff++;
    }

    packet->topic_size = i;

    return packet;
}

struct publish_packet * mqtt_publish_packet_create(struct fixed_header header, unsigned char * buff){
    struct publish_packet * packet = (struct publish_packet * ) malloc(sizeof(struct publish_packet));
    memset(packet, 0, sizeof(struct publish_packet));

    packet->publish_header = header;
    packet->retain = packet->publish_header.control_packet_2 & 1;
    packet->qos = 2 * (packet->publish_header.control_packet_2 >> 2 & 1) + (packet->publish_header.control_packet_2 >> 1 & 1);
    packet->dup = packet->publish_header.control_packet_2 >> 3 & 1;

    packet->variable_header.topic_name = hex_to_string(buff);
    buff += packet->variable_header.topic_name->string_len + 2;

    if(packet->qos > 0){
        packet->variable_header.identifier_MSB = *buff++;
        packet->variable_header.identifier_LSB = *buff++;
    }

    int str_len = packet->publish_header.remaining_length - packet->variable_header.topic_name->string_len - 2;

    packet->payload = (unsigned char *) malloc(sizeof(unsigned char) * (str_len + 1));
    memset(packet->payload, 0, sizeof(packet->payload));
    memcpy(packet->payload, buff, str_len);
    packet->payload_len = str_len;

    return packet;
}

pubrel_packet * mqtt_const_packet_create(struct fixed_header header, unsigned char * buff){
    pubrel_packet * packet = (pubrel_packet *) malloc(sizeof(pubrel_packet));
    memset(packet, 0, sizeof * packet);

    packet->const_header = header;

    packet->variable_header.byte1 = *buff++;
    packet->variable_header.byte2 = *buff;

    return packet;
}

int unsubscribe_topic_len(unsigned char * buff, int payload_len){
    int i = 0, j = 0, value = 0, tmp = 0;

    for(i = 0; *buff || *(buff + 1); i++){
        j = string_len(buff);
        buff += j + 2;
        if((tmp += j + 2) > payload_len){
            break;
        }
        value++;
    }

    return value;
}

struct unsubscribe_packet * mqtt_unsubscribe_packet_create(struct fixed_header header, unsigned char * buff){
    struct unsubscribe_packet * packet = (struct unsubscribe_packet *) malloc(sizeof * packet);
    memset(packet, 0, sizeof * packet);
    
    packet->unsubscribe_header = header;
    packet->variable_header.identifier_MSB = *buff++;
    packet->variable_header.identifier_LSB = *buff++;

    packet->un_topic_size = unsubscribe_topic_len(buff, packet->unsubscribe_header.remaining_length - 2);

    packet->payload = (mqtt_string **) malloc(sizeof(mqtt_string *) * (packet->un_topic_size + 1));
    memset(packet->payload, 0, sizeof(mqtt_string *) * packet->un_topic_size);

    for(int i = 0; i < packet->un_topic_size; i++){
        packet->payload[i] = hex_to_string(buff);
        buff += packet->payload[i]->string_len + 2;
    }

    return packet;
}

struct pingreq_packet * mqtt_pingreq_packet_create(struct fixed_header header){
    struct pingreq_packet * packet = (struct pingreq_packet *) malloc(sizeof * packet);
    memset(packet, 0, sizeof *packet);

    packet->pingreq_header = header;
    
    return packet;
}

struct disconnect_packet * mqtt_disconnect_packet_create(struct fixed_header header){
    struct disconnect_packet * packet = (struct disconnect_packet *) malloc(sizeof(struct disconnect_packet));
    memset(packet, 0, sizeof(struct disconnect_packet));

    packet->disconnect_header = header;

    return packet;
}

union mqtt_packet * mqtt_pack_decode(unsigned char * buff, int * packet_len)
{
    int i = 0, j;
    int multiplier = 1, value = 0;
    unsigned char * packet_buff = NULL;
    struct connect_packet * connack_packet;
    struct fixed_header header;
    union mqtt_packet * mqtt_packet;
    
    mqtt_packet = (union mqtt_packet *) malloc(sizeof(union mqtt_packet));
    memset(mqtt_packet, 0, sizeof(union mqtt_packet));
    
    header.control_packet_1 = buff[0] / 0x10;
    header.control_packet_2 = buff[0] % 0x10;

    do{
        value += (*++buff & 0x7F) * multiplier;

        multiplier *= 0x80;
        if(multiplier > 128 * 128 * 128){
            fprintf(stderr, "packet len error\n");
            return NULL;
        }
    }while( *buff & 0x80 );

    *packet_len = value + 2;
    header.remaining_length = value;

    if(value > 0){
        packet_buff = (unsigned char *) malloc(sizeof(unsigned char) * (value + 2));
        memset(packet_buff, 0, sizeof(unsigned char) * (value + 2));
        memmove(packet_buff, ++buff, value);

        printf_buff("packet_buff", buff, value);
    }

    switch (header.control_packet_1){
        case CONNECT:
            if((mqtt_packet->connect = mqtt_connect_packet_create(header, packet_buff)) == NULL){
                return NULL;
            }
            break;
        case PUBLISH:
            mqtt_packet->publish = mqtt_publish_packet_create(header, packet_buff);
            break;
        case PUBACK:
        case PUBREC:
        case PUBREL:
        case PUBCOMP:
            mqtt_packet->const_packet = mqtt_const_packet_create(header, packet_buff);
            break;
        case SUBSCRIBE:
            if((mqtt_packet->subscribe = mqtt_subscribe_packet_create(header, packet_buff)) == NULL){
                return NULL;
            }
            break;
        case UNSUBSCRIBE:
            mqtt_packet->unsubscribe = mqtt_unsubscribe_packet_create(header, packet_buff);
            break;
        case PINGREQ:
            mqtt_packet->pingreq = mqtt_pingreq_packet_create(header);
            break;
        case DISCONNECT:
            mqtt_packet->disconnect = mqtt_disconnect_packet_create(header);
            break;
        default:
            printf("other packet\n");
            return NULL;
            break;
    }

    return mqtt_packet;
}