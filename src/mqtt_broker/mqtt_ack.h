//
// Created by Ujjwal Krishnamurthi on 8/18/20.
//

#ifndef C_ENVIRONMENT_MQTT_ACK_H
#define C_ENVIRONMENT_MQTT_ACK_H

#include "mqtt_header.h"

struct mqtt_connack {
    union mqtt_header header;
    union {
        unsigned char byte;
        struct {
            unsigned session_present : 1;
            unsigned reserved : 7;
        } bits;
    };
    unsigned char rc;
};

struct mqtt_suback {
    union mqtt_header header;
    unsigned short pkt_id;
    unsigned short rcslen;
    unsigned char *rcs;
};

// Typedef-ed for all other general packet types
struct mqtt_ack {
    union mqtt_header header;
    unsigned short pkt_id;
};

typedef struct mqtt_ack mqtt_puback;
typedef struct mqtt_ack mqtt_pubrec;
typedef struct mqtt_ack mqtt_pubrel;
typedef struct mqtt_ack mqtt_pubcomp;
typedef struct mqtt_ack mqtt_unsuback;

#endif //C_ENVIRONMENT_MQTT_ACK_H
