#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>

#include <LUFA/Drivers/USB/USB.h>

#include "Descriptors.h"
#include "UART.h"
#include "Bridge.h"
#include "BridgeProtocol.h"

#define LINK_TIMEOUT_MS 120
#define REPORT_PERIOD_MS 4

static volatile xid_input_report_t g_input_report;
static volatile xid_output_report_t g_output_report;
static volatile bool g_output_pending = false;
static volatile bool g_usb_ready = false;
static volatile uint16_t g_millis = 0;

static bridge_packet_t g_last_packet;
static uint16_t g_last_packet_ms = 0;
static uint16_t g_last_report_ms = 0;
static bool g_have_packet = false;
static bool g_report_dirty = true;

static const uint8_t g_input_caps[20] = {
    0x00, 20,
    0xFF, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF
};

static const uint8_t g_output_caps[6] = {
    0x00, 6,
    0xFF, 0xFF,
    0xFF, 0xFF
};

static void setup_hardware(void);
static void process_bridge_uart(void);
static void process_interrupt_out(void);
static void maybe_send_interrupt_in(void);
static void send_rumble_feedback_uart(const xid_output_report_t* out);

ISR(TIMER0_COMPA_vect) {
    g_millis++;
}

static uint16_t millis16(void) {
    uint16_t now;
    uint8_t sreg = SREG;
    cli();
    now = g_millis;
    SREG = sreg;
    return now;
}

int main(void) {
    setup_hardware();
    bridge_reset_report((xid_input_report_t*)&g_input_report);
    memset((void*)&g_output_report, 0, sizeof(g_output_report));
    g_output_report.report_id = 0x00;
    g_output_report.length = sizeof(g_output_report);

    GlobalInterruptEnable();

    for (;;) {
        process_bridge_uart();
        process_interrupt_out();
        maybe_send_interrupt_in();
        USB_USBTask();
    }
}

static void setup_hardware(void) {
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    clock_prescale_set(clock_div_1);

    // 1 ms tick via Timer0 CTC.
    TCCR0A = (1 << WGM01);
    TCCR0B = (1 << CS01) | (1 << CS00);
    OCR0A = 249;
    TIMSK0 = (1 << OCIE0A);

    uart_init(1000000);
    USB_Init();
}

void EVENT_USB_Device_Connect(void) {
    g_usb_ready = false;
}

void EVENT_USB_Device_Disconnect(void) {
    g_usb_ready = false;
}

void EVENT_USB_Device_ConfigurationChanged(void) {
    bool ok = true;
    ok &= Endpoint_ConfigureEndpoint(XID_IN_EPADDR, EP_TYPE_INTERRUPT, XID_EPSIZE, 1);
    ok &= Endpoint_ConfigureEndpoint(XID_OUT_EPADDR, EP_TYPE_INTERRUPT, XID_EPSIZE, 1);
    g_usb_ready = ok;
}

void EVENT_USB_Device_ControlRequest(void) {
    // XID GET_DESCRIPTOR: bmRequestType=0xC1, bRequest=6, wValue=0x4200
    if (USB_ControlRequest.bmRequestType == 0xC1 &&
        USB_ControlRequest.bRequest == 6 &&
        (USB_ControlRequest.wValue >> 8) == 0x42) {

        Endpoint_ClearSETUP();
        Endpoint_Write_Control_PStream_LE(&XidDescriptor, sizeof(XID_Descriptor_t));
        Endpoint_ClearOUT();
        return;
    }

    // XID GET_REPORT input: bmRequestType=0xA1, bRequest=1, wValue=0x0100
    if (USB_ControlRequest.bmRequestType == 0xA1 &&
        USB_ControlRequest.bRequest == 0x01 &&
        USB_ControlRequest.wValue == 0x0100) {

        Endpoint_ClearSETUP();
        Endpoint_Write_Control_Stream_LE((const void*)&g_input_report, sizeof(g_input_report));
        Endpoint_ClearOUT();
        return;
    }

    // XID GET_CAPABILITIES input: bmRequestType=0xC1, bRequest=1, wValue=0x0100
    if (USB_ControlRequest.bmRequestType == 0xC1 &&
        USB_ControlRequest.bRequest == 0x01 &&
        USB_ControlRequest.wValue == 0x0100) {

        Endpoint_ClearSETUP();
        Endpoint_Write_Control_PStream_LE(g_input_caps, sizeof(g_input_caps));
        Endpoint_ClearOUT();
        return;
    }

    // XID GET_CAPABILITIES output: bmRequestType=0xC1, bRequest=1, wValue=0x0200
    if (USB_ControlRequest.bmRequestType == 0xC1 &&
        USB_ControlRequest.bRequest == 0x01 &&
        USB_ControlRequest.wValue == 0x0200) {

        Endpoint_ClearSETUP();
        Endpoint_Write_Control_PStream_LE(g_output_caps, sizeof(g_output_caps));
        Endpoint_ClearOUT();
        return;
    }

    // XID SET_REPORT output (rumble): bmRequestType=0x21, bRequest=9, wValue=0x0200
    if (USB_ControlRequest.bmRequestType == 0x21 &&
        USB_ControlRequest.bRequest == 9 &&
        USB_ControlRequest.wValue == 0x0200 &&
        USB_ControlRequest.wLength == sizeof(g_output_report)) {

        Endpoint_ClearSETUP();
        Endpoint_Read_Control_Stream_LE((void*)&g_output_report, sizeof(g_output_report));
        Endpoint_ClearIN();
        g_output_pending = true;
        send_rumble_feedback_uart((const xid_output_report_t*)&g_output_report);
        return;
    }
}

static void process_bridge_uart(void) {
    static uint8_t raw[sizeof(bridge_packet_t)];
    static uint8_t idx = 0;

    while (uart_rx_available()) {
        const uint8_t b = uart_rx_read();

        if (idx == 0) {
            if (b != BRIDGE_PKT_MAGIC) {
                continue;
            }
            raw[idx++] = b;
            continue;
        }

        raw[idx++] = b;

        if (idx >= sizeof(raw)) {
            idx = 0;
            bridge_packet_t pkt;
            if (bridge_packet_decode(raw, &pkt)) {
                g_last_packet = pkt;
                g_last_packet_ms = millis16();
                g_have_packet = true;
                bridge_map_packet_to_xid(&g_last_packet, (xid_input_report_t*)&g_input_report);
                g_report_dirty = true;
            }
        }
    }

    if (g_have_packet) {
        const uint16_t now = millis16();
        if ((uint16_t)(now - g_last_packet_ms) > LINK_TIMEOUT_MS) {
            g_have_packet = false;
            bridge_reset_report((xid_input_report_t*)&g_input_report);
            g_report_dirty = true;
        }
    }
}

static void maybe_send_interrupt_in(void) {
    if (!g_usb_ready || USB_DeviceState != DEVICE_STATE_Configured) {
        return;
    }

    const uint16_t now = millis16();
    if (!g_report_dirty && ((uint16_t)(now - g_last_report_ms) < REPORT_PERIOD_MS)) {
        return;
    }

    Endpoint_SelectEndpoint(XID_IN_EPADDR);
    if (!Endpoint_IsINReady()) {
        return;
    }

    Endpoint_Write_Stream_LE((const void*)&g_input_report, sizeof(g_input_report), NULL);
    Endpoint_ClearIN();
    g_last_report_ms = now;
    g_report_dirty = false;
}

static void process_interrupt_out(void) {
    if (!g_usb_ready || USB_DeviceState != DEVICE_STATE_Configured) {
        return;
    }

    Endpoint_SelectEndpoint(XID_OUT_EPADDR);
    if (!Endpoint_IsOUTReceived()) {
        return;
    }

    if (Endpoint_IsReadWriteAllowed() && Endpoint_BytesInEndpoint() >= sizeof(g_output_report)) {
        Endpoint_Read_Stream_LE((void*)&g_output_report, sizeof(g_output_report), NULL);
        g_output_pending = true;
        send_rumble_feedback_uart((const xid_output_report_t*)&g_output_report);
    }

    Endpoint_ClearOUT();
}

static uint8_t crc8_local(const uint8_t* data, uint8_t len) {
    return bridge_crc8(data, len);
}

static void send_rumble_feedback_uart(const xid_output_report_t* out) {
    uint8_t frame[6];
    frame[0] = 0x5A;
    frame[1] = (uint8_t)(out->left_motor >> 8);
    frame[2] = (uint8_t)(out->right_motor >> 8);
    frame[3] = 0x00;
    frame[4] = 0x00;
    frame[5] = crc8_local(frame, 5);
    uart_tx_write_buf(frame, sizeof(frame));
}
