#pragma once

#include <LUFA/Drivers/USB/USB.h>
#include <avr/pgmspace.h>
#include <stdint.h>

enum InterfaceDescriptors_t {
    INTERFACE_ID_XID = 0,
};

enum StringDescriptors_t {
    STRING_ID_Language = 0,
    STRING_ID_Manufacturer = 1,
    STRING_ID_Product = 2,
};

#define XID_IN_EPADDR  (ENDPOINT_DIR_IN  | 1)
#define XID_OUT_EPADDR (ENDPOINT_DIR_OUT | 2)
#define XID_EPSIZE     32

typedef struct __attribute__((packed)) {
    USB_Descriptor_Configuration_Header_t Config;
    USB_Descriptor_Interface_t            XidInterface;
    USB_Descriptor_Endpoint_t             XidDataInEndpoint;
    USB_Descriptor_Endpoint_t             XidDataOutEndpoint;
} USB_Descriptor_Configuration_t;

typedef struct __attribute__((packed)) {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t bcdXid;
    uint8_t  bType;
    uint8_t  bSubType;
    uint8_t  bMaxInputReportSize;
    uint8_t  bMaxOutputReportSize;
    uint16_t wAlternateProductIds[4];
} XID_Descriptor_t;

extern const USB_Descriptor_Device_t PROGMEM DeviceDescriptor;
extern const USB_Descriptor_Configuration_t PROGMEM ConfigurationDescriptor;
extern const XID_Descriptor_t PROGMEM XidDescriptor;

uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint8_t wIndex,
                                    const void** const DescriptorAddress)
                                    ATTR_WARN_UNUSED_RESULT ATTR_NON_NULL_PTR_ARG(3);
