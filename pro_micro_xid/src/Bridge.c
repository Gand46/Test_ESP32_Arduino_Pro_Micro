#include "Bridge.h"

// Bluepad32 typical raw values seen in examples / issue snippets.
#define BP32_DPAD_UP       0x01
#define BP32_DPAD_DOWN     0x02
#define BP32_DPAD_RIGHT    0x04
#define BP32_DPAD_LEFT     0x08

#define BP32_BTN_A         0x0001
#define BP32_BTN_B         0x0002
#define BP32_BTN_X         0x0004
#define BP32_BTN_Y         0x0008
#define BP32_BTN_L1        0x0010
#define BP32_BTN_R1        0x0020
#define BP32_BTN_THUMBL    0x0040
#define BP32_BTN_THUMBR    0x0080

#define BP32_MISC_SYSTEM   0x01
#define BP32_MISC_BACK     0x02
#define BP32_MISC_START    0x04

static int16_t scale_axis_to_xid(int16_t v) {
    long s = (long)v * 32767L / 512L;
    if (s > 32767L) {
        s = 32767L;
    } else if (s < -32767L) {
        s = -32767L;
    }
    return (int16_t)s;
}

static uint8_t scale_trigger_to_u8(uint16_t v) {
    uint16_t s = (uint16_t)((uint32_t)v * 255UL / 1023UL);
    if (s > 255U) {
        s = 255U;
    }
    return (uint8_t)s;
}

void bridge_reset_report(xid_input_report_t* r) {
    r->report_id = 0x00;
    r->length = sizeof(xid_input_report_t);
    r->digital1 = 0x00;
    r->digital2 = 0x00;
    r->analog_a = 0x00;
    r->analog_b = 0x00;
    r->analog_x = 0x00;
    r->analog_y = 0x00;
    r->analog_black = 0x00;
    r->analog_white = 0x00;
    r->lt = 0x00;
    r->rt = 0x00;
    r->lx = 0;
    r->ly = 0;
    r->rx = 0;
    r->ry = 0;
}

void bridge_map_packet_to_xid(const bridge_packet_t* p, xid_input_report_t* r) {
    bridge_reset_report(r);

    if ((p->flags & 0x01u) == 0u) {
        return;
    }

    if (p->dpad & BP32_DPAD_UP)    r->digital1 |= (1u << 0);
    if (p->dpad & BP32_DPAD_DOWN)  r->digital1 |= (1u << 1);
    if (p->dpad & BP32_DPAD_LEFT)  r->digital1 |= (1u << 2);
    if (p->dpad & BP32_DPAD_RIGHT) r->digital1 |= (1u << 3);
    if (p->misc & BP32_MISC_START) r->digital1 |= (1u << 4);
    if (p->misc & BP32_MISC_BACK)  r->digital1 |= (1u << 5);
    if (p->buttons & BP32_BTN_THUMBL) r->digital1 |= (1u << 6);
    if (p->buttons & BP32_BTN_THUMBR) r->digital1 |= (1u << 7);

    r->analog_a = (p->buttons & BP32_BTN_A) ? 0xFFu : 0x00u;
    r->analog_b = (p->buttons & BP32_BTN_B) ? 0xFFu : 0x00u;
    r->analog_x = (p->buttons & BP32_BTN_X) ? 0xFFu : 0x00u;
    r->analog_y = (p->buttons & BP32_BTN_Y) ? 0xFFu : 0x00u;

    // Base policy: L1/R1 -> Black/White. Make configurable later if needed.
    r->analog_black = (p->buttons & BP32_BTN_L1) ? 0xFFu : 0x00u;
    r->analog_white = (p->buttons & BP32_BTN_R1) ? 0xFFu : 0x00u;

    // Bluepad32 names left/right triggers as brake/throttle in many examples.
    r->lt = scale_trigger_to_u8(p->brake);
    r->rt = scale_trigger_to_u8(p->throttle);

    r->lx = scale_axis_to_xid(p->lx);
    r->ly = (int16_t)-scale_axis_to_xid(p->ly);
    r->rx = scale_axis_to_xid(p->rx);
    r->ry = (int16_t)-scale_axis_to_xid(p->ry);
}
