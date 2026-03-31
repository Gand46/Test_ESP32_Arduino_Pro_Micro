#include "BridgeProtocol.h"
#include <string.h>

uint8_t bridge_crc8(const uint8_t* data, uint8_t len) {
    uint8_t crc = 0x00;
    for (uint8_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (uint8_t b = 0; b < 8; ++b) {
            crc = (crc & 0x80u) ? (uint8_t)((crc << 1) ^ 0x07u) : (uint8_t)(crc << 1);
        }
    }
    return crc;
}

bool bridge_packet_decode(const uint8_t* raw, bridge_packet_t* out) {
    memcpy(out, raw, sizeof(*out));

    if (out->magic != BRIDGE_PKT_MAGIC) {
        return false;
    }
    if (out->version != BRIDGE_PKT_VERSION) {
        return false;
    }

    const uint8_t got = out->crc;
    out->crc = 0;
    const uint8_t calc = bridge_crc8((const uint8_t*)out, (uint8_t)(sizeof(*out) - 1));
    out->crc = got;
    return got == calc;
}
