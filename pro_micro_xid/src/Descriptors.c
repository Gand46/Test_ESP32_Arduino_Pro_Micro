#include "Descriptors.h"

#define XID_VENDOR_ID  0xFFFF
#define XID_PRODUCT_ID 0xFFFF
#define XID_RELEASE_NO VERSION_BCD(0, 1, 0)

const USB_Descriptor_Device_t PROGMEM DeviceDescriptor = {
    .Header                 = {.Size = sizeof(USB_Descriptor_Device_t), .Type = DTYPE_Device},
    .USBSpecification       = VERSION_BCD(2, 0, 0),
    .Class                  = USB_CSCP_VendorSpecificClass,
    .SubClass               = USB_CSCP_NoDeviceSubclass,
    .Protocol               = USB_CSCP_NoDeviceProtocol,
    .Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,
    .VendorID               = XID_VENDOR_ID,
    .ProductID              = XID_PRODUCT_ID,
    .ReleaseNumber          = XID_RELEASE_NO,
    .ManufacturerStrIndex   = STRING_ID_Manufacturer,
    .ProductStrIndex        = STRING_ID_Product,
    .SerialNumStrIndex      = NO_DESCRIPTOR,
    .NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

const USB_Descriptor_Configuration_t PROGMEM ConfigurationDescriptor = {
    .Config = {
        .Header                 = {.Size = sizeof(USB_Descriptor_Configuration_Header_t), .Type = DTYPE_Configuration},
        .TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t),
        .TotalInterfaces        = 1,
        .ConfigurationNumber    = 1,
        .ConfigurationStrIndex  = NO_DESCRIPTOR,
        .ConfigAttributes       = (USB_CONFIG_ATTR_RESERVED),
        .MaxPowerConsumption    = USB_CONFIG_POWER_MA(100)
    },

    .XidInterface = {
        .Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},
        .InterfaceNumber        = INTERFACE_ID_XID,
        .AlternateSetting       = 0,
        .TotalEndpoints         = 2,
        .Class                  = USB_CSCP_VendorSpecificClass,
        .SubClass               = USB_CSCP_NoDeviceSubclass,
        .Protocol               = USB_CSCP_NoDeviceProtocol,
        .InterfaceStrIndex      = NO_DESCRIPTOR
    },

    .XidDataInEndpoint = {
        .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
        .EndpointAddress        = XID_IN_EPADDR,
        .Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
        .EndpointSize           = XID_EPSIZE,
        .PollingIntervalMS      = 4
    },

    .XidDataOutEndpoint = {
        .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
        .EndpointAddress        = XID_OUT_EPADDR,
        .Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
        .EndpointSize           = XID_EPSIZE,
        .PollingIntervalMS      = 4
    }
};

const XID_Descriptor_t PROGMEM XidDescriptor = {
    .bLength = sizeof(XID_Descriptor_t),
    .bDescriptorType = 0x42,
    .bcdXid = VERSION_BCD(1, 0, 0),
    .bType = 0x01,
    .bSubType = 0x02,
    .bMaxInputReportSize = 20,
    .bMaxOutputReportSize = 6,
    .wAlternateProductIds = {0, 0, 0, 0}
};

const USB_Descriptor_String_t PROGMEM LanguageString = USB_STRING_DESCRIPTOR_ARRAY(LANGUAGE_ID_ENG);
const USB_Descriptor_String_t PROGMEM ManufacturerString = USB_STRING_DESCRIPTOR(L"Custom");
const USB_Descriptor_String_t PROGMEM ProductString = USB_STRING_DESCRIPTOR(L"XID Bridge");

uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint8_t wIndex,
                                    const void** const DescriptorAddress) {
    (void)wIndex;

    const uint8_t descriptorType = (uint8_t)(wValue >> 8);
    const uint8_t descriptorNumber = (uint8_t)(wValue & 0xFF);

    const void* address = NULL;
    uint16_t size = NO_DESCRIPTOR;

    switch (descriptorType) {
        case DTYPE_Device:
            address = &DeviceDescriptor;
            size = sizeof(DeviceDescriptor);
            break;
        case DTYPE_Configuration:
            address = &ConfigurationDescriptor;
            size = sizeof(ConfigurationDescriptor);
            break;
        case DTYPE_String:
            switch (descriptorNumber) {
                case STRING_ID_Language:
                    address = &LanguageString;
                    size = pgm_read_byte(&LanguageString.Header.Size);
                    break;
                case STRING_ID_Manufacturer:
                    address = &ManufacturerString;
                    size = pgm_read_byte(&ManufacturerString.Header.Size);
                    break;
                case STRING_ID_Product:
                    address = &ProductString;
                    size = pgm_read_byte(&ProductString.Header.Size);
                    break;
            }
            break;
        case 0x42:
            address = &XidDescriptor;
            size = sizeof(XidDescriptor);
            break;
    }

    *DescriptorAddress = address;
    return size;
}
