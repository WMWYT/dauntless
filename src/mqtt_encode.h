#ifndef MQTT_ENCODE_H
#define MQTT_ENCODE_H

void ML_encode(int x, unsigned char * MSB, unsigned char * LSB);

char * mqtt_conncet_encode(unsigned char c_flag, int keep_alive \
                            , char * client_id, char * will_topic, char * will_message \
                            , char * user_name, char * password);

char * mqtt_connack_encode(int acknowledge_flag, int return_code);

int mqtt_publish_encode_qos_0(unsigned char * topic, unsigned char * payload, char * buff);

int mqtt_publish_encode_qos_1_2(unsigned char * topic, int qos, unsigned char id_M, unsigned char id_L, unsigned char * payload, char * buff);

char * mqtt_publish_qos_encode(int control_type, int flag, int identifier_MSB, int identifier_LSB);

int mqtt_subscribe_encode(char ** topic, int * qos, int topic_size, int identifier, unsigned char * buff);

char * mqtt_pingresp_encode();

char * mqtt_suback_encode(int i_M, int i_L, int topic_size, int * return_code);

char * mqtt_unsuback_encode(int i_M, int i_L);

#endif