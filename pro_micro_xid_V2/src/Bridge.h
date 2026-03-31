#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "BridgeProtocol.h"

typedef struct __attribute__((packed)) {
    uint8_t report_id;
    uint8_t length;
    uint8_t digital1;
    uint8_t digital2;
    uint8_t analog_a;
    uint8_t analog_b;
    uint8_t analog_x;
    uint8_t analog_y;
    uint8_t analog_black;
    uint8_t analog_white;
    uint8_t lt;
    uint8_t rt;
    int16_t lx;
    int16_t ly;
    int16_t rx;
    int16_t ry;
} xid_input_report_t;

typedef struct __attribute__((packed)) {
    uint8_t report_id;
    uint8_t length;
    uint16_t left_motor;
    uint16_t right_motor;
} xid_output_report_t;

void bridge_reset_report(xid_input_report_t* r);
void bridge_map_packet_to_xid(const bridge_packet_t* p, xid_input_report_t* r);
