//
// Created by Ujjwal Krishnamurthi on 8/18/20.
//

#ifndef C_ENVIRONMENT_MQTT_H
#define C_ENVIRONMENT_MQTT_H

#include <stdio.h>
#include "mqtt_pack.h"
#include "mqtt_ack.h"

union mqtt_packet {
    struct mqtt_ack ack;
    union mqtt_header header;
    struct mqtt_connect connect;
    struct mqtt_connack connack;
    struct mqtt_suback suback;
    struct mqtt_publish publish;
    struct mqtt_subscribe subscribe;
    struct mqtt_unsubscribe unsubscribe;
};

// Utility functions for packing/marshalling and unpacking/unmarshalling the packets
int mqtt_encode_length(unsigned char *, size_t);
unsigned long long mqtt_decode_length(const unsigned char **);
int unpack_mqtt_packet(const unsigned char *, union mqtt_packet *);
unsigned char *pack_mqtt_packet(const union mqtt_packet *, unsigned);

// Utility functions to build packets and release memory
union mqtt_header *mqtt_packet_header(unsigned char);
struct mqtt_ack *mqtt_packet_ack(unsigned char , unsigned short);
struct mqtt_connack *mqtt_packet_connack(unsigned char, unsigned char, unsigned char);
struct mqtt_suback *mqtt_packet_suback(unsigned char, unsigned short, unsigned char *, unsigned short);
struct mqtt_publish *mqtt_packet_publish(unsigned char, unsigned short, size_t, unsigned char *, size_t, unsigned char *);
void mqtt_packet_release(union mqtt_packet *, unsigned);

#endif //C_ENVIRONMENT_MQTT_H
