#include <fx2regs.h>
#include <fx2debug.h>
#include <fx2ints.h>
#include <fx2usb.h>
#include <fx2lib.h>
#include <fx2usbmassstor.h>

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "hid.h"
#include "io.h"
#include "piuio.h"
#include "keyboard.h"

volatile bool pending_ep1_in;

usb_desc_device_c usb_device = {
    .bLength = sizeof(struct usb_desc_device),
    .bDescriptorType = USB_DESC_DEVICE,
    .bcdUSB = 0x0110,
    .bDeviceClass = USB_DEV_CLASS_PER_INTERFACE,
    .bDeviceSubClass = USB_DEV_SUBCLASS_PER_INTERFACE,
    .bDeviceProtocol = USB_DEV_PROTOCOL_PER_INTERFACE,
    .bMaxPacketSize0 = 64,
    .idVendor = KEYBOARD_VID,
    .idProduct = KEYBOARD_PID,
    .bcdDevice = 0x0001,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 0,
    .bNumConfigurations = 1,
};

usb_desc_interface_c keyboard_interface = {
    .bLength = sizeof(struct usb_desc_interface),
    .bDescriptorType = USB_DESC_INTERFACE,
    .bInterfaceNumber = 0,
    .bAlternateSetting = 0,
    .bNumEndpoints = 1,
    .bInterfaceClass = HID_CSCP_HIDClass,
    .bInterfaceSubClass = HID_CSCP_NonBootSubclass,
    .bInterfaceProtocol = HID_CSCP_NonBootProtocol,
    .iInterface = 0,
};

usb_hid_descriptor_c keyboard_hid_interface = {
    .bLength = sizeof(struct usb_hid_descriptor),
    .bDescriptorType = HID_TYPE_HID,
    .bcdHID = 0x0111,
    .bCountryCode = 0,
    .bNumDescriptors = 1,
    .bHIDReportType = HID_TYPE_HID_REPORT,
    .wDescriptorLength = sizeof(HID_Keyboard_Report),
};

usb_desc_endpoint_c keyboard_int_in = {
    .bLength = sizeof(struct usb_desc_endpoint),
    .bDescriptorType = USB_DESC_ENDPOINT,
    .bEndpointAddress = HID_KEYBOARD_EPINADDR,
    .bmAttributes = USB_XFER_INTERRUPT | USB_SYNC_NONE | USB_USAGE_DATA,
    .wMaxPacketSize = 32,
    .bInterval = 1,
};

usb_configuration_c usb_config = {
    {
        .bLength = sizeof(struct usb_desc_configuration),
        .bDescriptorType = USB_DESC_CONFIGURATION,
        .bNumInterfaces = 1,
        .bConfigurationValue = 1,
        .iConfiguration = 0,
        .bmAttributes = USB_ATTR_RESERVED_1 | USB_ATTR_REMOTE_WAKEUP,
        .bMaxPower = 50,
    },
    {
        {.interface = &keyboard_interface},
        {.generic = (struct usb_desc_generic *)&keyboard_hid_interface},
        {.endpoint = &keyboard_int_in},
        {0},
    },
};

usb_configuration_set_c usb_configs[] = {
    &usb_config,
};

usb_ascii_string_c usb_strings[] = {
    [0] = "icedragon.io",
    [1] = "fusion-keyboard",
    // TODO: generate serial number
    [2] = "000000000000",
};

usb_descriptor_set_c usb_descriptor_set = {
    .device = &usb_device,
    .config_count = ARRAYSIZE(usb_configs),
    .configs = usb_configs,
    .string_count = ARRAYSIZE(usb_strings),
    .strings = usb_strings,
};

// TODO: Sync polling to the SOF event.

// callback for control transfer requests.
void handle_usb_setup(__xdata struct usb_req_setup *req)
{
    if ((req->wValue >> 8) == HID_TYPE_HID_REPORT)
    {
        // can't copy to EP0BUF directly as this report is too large.
        SETUP_EP0_IN_DATA(&HID_Keyboard_Report[0], sizeof(HID_Keyboard_Report));
    }
    else if ((req->wValue >> 8) == HID_TYPE_HID)
    {
        SETUP_EP0_IN_DATA(&keyboard_hid_interface, sizeof(keyboard_hid_interface));
    }
    else
    {
        STALL_EP0();
    }
}

void isr_IBN(void) __interrupt
{
    pending_ep1_in = true;
    CLEAR_USB_IRQ();
    NAKIRQ = _IBN;
    IBNIRQ = _IBNI_EP1;
}

int main(void)
{
    // Run core at 48 MHz fCLK.
    CPUCS = _CLKSPD1;
    // Use newest chip features.
    REVCTL = _ENH_PKT | _DYN_OUT;

    // NAK all transfers.
    SYNCDELAY;
    FIFORESET = _NAKALL;

    // EP1 is configured INTERRUPT IN.
    EP1INCFG = _VALID | _TYPE0 | _TYPE1;
    EP1INCS = 0;

    // EP1 is not used.
    EP1OUTCFG &= ~_VALID;

    // EP4/8 are not used.
    EP4CFG &= ~_VALID;
    EP8CFG &= ~_VALID;

    // Enable IN-BULK-NAK interrupt for EP1.
    IBNIE = _IBNI_EP1;
    NAKIE = _IBN;

    // Reset and prime EP1, and reset EP1.
    SYNCDELAY;
    FIFORESET = _NAKALL | 1;
    SYNCDELAY;
    OUTPKTEND = _SKIP | 1;
    SYNCDELAY;
    OUTPKTEND = _SKIP | 1;
    SYNCDELAY;
    FIFORESET = _NAKALL | 1;
    SYNCDELAY;
    FIFORESET = 0;

    // disables all interrupts
    EA = 0;

    // sets up all of our pins and timer.
    init_io(true);

    // Enable interrupts
    EA = 1;

    // true will reenumerate on boot.
    usb_init(true);

    keyboard_init();

    while (1)
    {
        io_task();

        // data flowing out of device.
        if (pending_ep1_in)
        {
            // returns true on a change in state.
            if (keyboard_genreport())
            {
                EP1INBC = HID_KEYBOARD_REPORT_SIZE;

                pending_ep1_in = false;
            }
        }
    }
}
