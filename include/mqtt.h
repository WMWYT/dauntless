#ifndef MQTT_H
#define MQTT_H

enum control_packet{
    CONNECT = 0x01,
    CONNACK,
    PUBLISH,
    PUBACK,
    PUBREC,
    PUBREL,
    PUBCOMP,
    SUBSCRIBE,
    SUBACK,
    UNSUBSCRIBE,
    UNSUBACK,
    PINGREQ,
    PINGRESP,
    DISCONNECT
};

enum connect_return_codes{
    CONNECT_ACCEPTED,
    CONNECT_ERROR_VERSION,
    CONNECT_ERROR_IDENTIFIER,
    CONNECT_ERROR_SERVER,
    CONNECT_ERROR_USER_OR_PASSWORD,
    CONNECT_ERROR_AUTHORIZED
};

enum subscribe_return_codes{
    QOS_0,
    QOS_1,
    QOS_2,
    FAILURE = 0x80
};

enum sevice_error_code{
    TIME_OUT = 1,
};

struct fixed_header
{
    unsigned char control_packet_1;
    unsigned char control_packet_2;
    int remaining_length;
};

typedef struct fixed_header fixed_header;

struct mqtt_string
{
    unsigned char length_MSB;
    unsigned char length_LSB;
    int string_len;
    unsigned char * string;
};

typedef struct mqtt_string mqtt_string;

struct connect_packet{
    fixed_header connect_header;
    struct {
        mqtt_string * protocol_name;
        
        unsigned char protocol_level;

        unsigned char connect_flags;

        unsigned char keep_alive_MSB;
        unsigned char keep_alive_LSB;
    }variable_header;

    struct {
        mqtt_string * client_id;
        mqtt_string * will_topic;
        mqtt_string * will_payload;
        mqtt_string * user_name;
        mqtt_string * password;
    }payload;

    int error_code;
};

struct subscribe_payload {
    mqtt_string * topic_filter;
    unsigned char qos;
};

typedef struct subscribe_payload subscribe_payload; 

struct subscribe_packet{
    fixed_header subscribe_header;
    struct {
        unsigned char identifier_MSB;
        unsigned char identifier_LSB;
    }variable_header;

    subscribe_payload * payload;

    int topic_size;
};

struct mqtt_const_packet{
    fixed_header const_header;

    struct{
        unsigned char byte1;
        unsigned char byte2;
    }variable_header;
};

struct suback_packet{
    fixed_header suback_header;

    struct {
        unsigned char identifier_MSB;
        unsigned char identifier_LSB;
    }variable_header;

    int * return_codes;
    int return_code_size;
};

struct publish_packet{
    fixed_header publish_header;

    int dup;
    int qos;
    int retain;

    struct {
        mqtt_string * topic_name;
        unsigned char identifier_MSB;
        unsigned char identifier_LSB;
    }variable_header;

    int payload_len;
    unsigned char * payload;
};

struct unsubscribe_packet {
    fixed_header unsubscribe_header;

    struct {
    unsigned char identifier_MSB;
    unsigned char identifier_LSB;
    } variable_header;

    mqtt_string ** payload;

    int un_topic_size;
};

struct pingreq_packet{
    fixed_header pingreq_header;
};

struct disconnect_packet{
    fixed_header disconnect_header;
};

typedef struct mqtt_const_packet connack_packet;
typedef struct mqtt_const_packet puback_packet;
typedef struct mqtt_const_packet pubrec_packet;
typedef struct mqtt_const_packet pubrel_packet;
typedef struct mqtt_const_packet pubcomp_packet;
typedef struct mqtt_const_packet unsuback_packet;

union mqtt_packet{
    struct connect_packet * connect;
    struct publish_packet * publish;
    struct mqtt_const_packet * const_packet;
    struct subscribe_packet * subscribe;
    struct unsubscribe_packet * unsubscribe;
    struct pingreq_packet * pingreq;
    struct disconnect_packet * disconnect;
};

#endif