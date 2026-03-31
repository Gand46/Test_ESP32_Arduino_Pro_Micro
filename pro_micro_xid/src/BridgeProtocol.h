#pragma once

#include <stdint.h>
#include <stdbool.h>

#define BRIDGE_PKT_MAGIC   0xA5
#define BRIDGE_PKT_VERSION 0x01

typedef struct __attribute__((packed)) {
    uint8_t  magic;
    uint8_t  version;
    uint8_t  seq;
    uint8_t  flags;

    uint8_t  dpad;
    uint16_t buttons;
    uint8_t  misc;

    uint16_t brake;
    uint16_t throttle;

    int16_t  lx;
    int16_t  ly;
    int16_t  rx;
    int16_t  ry;

    uint8_t  crc;
} bridge_packet_t;

uint8_t bridge_crc8(const uint8_t* data, uint8_t len);
bool bridge_packet_decode(const uint8_t* raw, bridge_packet_t* out);
