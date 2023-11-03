#ifndef MQTT_DECODE_H
#define MQTT_DECODE_H

int string_len(unsigned char * buff);
union mqtt_packet * mqtt_pack_decode(unsigned char * buff, int * packet_len);

#endif