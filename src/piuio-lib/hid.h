#ifndef _HID_H_
#define _HID_H_

#include <stdint.h>

#define HID_TYPE_HID 0x21
#define HID_TYPE_HID_REPORT 0x22

#define HID_CSCP_HIDClass 0x03
#define HID_CSCP_NonBootSubclass 0x00
#define HID_CSCP_NonBootProtocol 0x00

struct usb_hid_descriptor
{
    // standard header
    uint8_t bLength;
    uint8_t bDescriptorType;

    // hid specific
    uint16_t bcdHID;
    uint8_t bCountryCode;
    uint8_t bNumDescriptors;
    uint8_t bHIDReportType;
    uint16_t wDescriptorLength;
};

typedef __code const struct usb_hid_descriptor
    usb_hid_descriptor_c;

#endif