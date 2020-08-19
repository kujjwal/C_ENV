//
// Created by Ujjwal Krishnamurthi on 8/18/20.
//

#ifndef C_ENVIRONMENT_MQTT_HEADER_H
#define C_ENVIRONMENT_MQTT_HEADER_H

#define MQTT_HEADER_LEN_BYTES 2
#define MQTT_ACK_LEN_BYTES 4

// Unpacking enum to define preprocessing for standardization across compilers
#define AT_MOST_ONCE 0u
#define AT_LEAST_ONCE 1u
#define EXACTLY_ONCE 2u

/**
 * Stub bytes, useful for generic replies, these represent the first byte in
 * the fixed header
 */
#define CONNACK_BYTE  0x20
#define PUBLISH_BYTE  0x30
#define PUBACK_BYTE   0x40
#define PUBREC_BYTE   0x50
#define PUBREL_BYTE   0x60
#define PUBCOMP_BYTE  0x70
#define SUBACK_BYTE   0x90
#define UNSUBACK_BYTE 0xB0
#define PINGRESP_BYTE 0xD0

/* Message types */
enum packet_type {
    CONNECT     = 1,
    CONNACK     = 2,
    PUBLISH     = 3,
    PUBACK      = 4,
    PUBREC      = 5,
    PUBREL      = 6,
    PUBCOMP     = 7,
    SUBSCRIBE   = 8,
    SUBACK      = 9,
    UNSUBSCRIBE = 10,
    UNSUBACK    = 11,
    PINGREQ     = 12,
    PINGRESP    = 13,
    DISCONNECT  = 14
};

//enum qos_level { AT_MOST_ONCE = 0u, AT_LEAST_ONCE = 1u, EXACTLY_ONCE = 2u };

union mqtt_header {
    unsigned char byte;
    struct {
        unsigned retain : 1;
        unsigned qos : 2;
        unsigned dup : 3;
        unsigned type: 4;
    } bits;
};

#endif //C_ENVIRONMENT_MQTT_HEADER_H
