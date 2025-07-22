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
#include "lxio.h"

volatile bool pending_ep1_in;

usb_desc_device_c usb_device = {
    .bLength = sizeof(struct usb_desc_device),
    .bDescriptorType = USB_DESC_DEVICE,
    .bcdUSB = 0x0110,
    .bDeviceClass = USB_DEV_CLASS_PER_INTERFACE,
    .bDeviceSubClass = USB_DEV_SUBCLASS_PER_INTERFACE,
    .bDeviceProtocol = USB_DEV_PROTOCOL_PER_INTERFACE,
    .bMaxPacketSize0 = 64,
    .idVendor = LXIO_VID,
    .idProduct = LXIO_PID_V1,
    .bcdDevice = 0x0001,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 0,
    .bNumConfigurations = 1,
};

usb_desc_interface_c lxio_interface = {
    .bLength = sizeof(struct usb_desc_interface),
    .bDescriptorType = USB_DESC_INTERFACE,
    .bInterfaceNumber = 0,
    .bAlternateSetting = 0,
    .bNumEndpoints = 2,
    .bInterfaceClass = HID_CSCP_HIDClass,
    .bInterfaceSubClass = HID_CSCP_NonBootSubclass,
    .bInterfaceProtocol = HID_CSCP_NonBootProtocol,
    .iInterface = 1,
};

usb_hid_descriptor_c lxio_hid_interface = {
    .bLength = sizeof(struct usb_hid_descriptor),
    .bDescriptorType = HID_TYPE_HID,
    .bcdHID = 0x0111,
    .bCountryCode = 0,
    .bNumDescriptors = 1,
    .bHIDReportType = HID_TYPE_HID_REPORT,
    .wDescriptorLength = sizeof(HID_LXIO_Report),
};

usb_desc_endpoint_c lxio_int_in = {
    .bLength = sizeof(struct usb_desc_endpoint),
    .bDescriptorType = USB_DESC_ENDPOINT,
    .bEndpointAddress = HID_LXIO_EPINADDR,
    .bmAttributes = USB_XFER_INTERRUPT | USB_SYNC_NONE | USB_USAGE_DATA,
    .wMaxPacketSize = HID_LXIO_EPSIZE,
    .bInterval = 1,
};

usb_desc_endpoint_c lxio_int_out = {
    .bLength = sizeof(struct usb_desc_endpoint),
    .bDescriptorType = USB_DESC_ENDPOINT,
    .bEndpointAddress = HID_LXIO_EPOUTADDR,
    .bmAttributes = USB_XFER_INTERRUPT | USB_SYNC_NONE | USB_USAGE_DATA,
    .wMaxPacketSize = HID_LXIO_EPSIZE,
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
        {.interface = &lxio_interface},
        {.generic = (struct usb_desc_generic *)&lxio_hid_interface},
        {.endpoint = &lxio_int_in},
        {.endpoint = &lxio_int_out},
        {0},
    },
};

usb_configuration_set_c usb_configs[] = {
    &usb_config,
};

usb_ascii_string_c usb_strings[] = {
    [0] = "icedragon.io",
    [1] = "fusion-lxio",
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
        memcpy(&EP0BUF[0], &HID_LXIO_Report[0], sizeof(HID_LXIO_Report));
        SETUP_EP0_BUF((uint8_t)sizeof(HID_LXIO_Report));
    }
    else if ((req->wValue >> 8) == HID_TYPE_HID)
    {
        SETUP_EP0_IN_DATA(&lxio_hid_interface, sizeof(lxio_hid_interface));
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

    // EP2 is configured as 512-byte double buffed INTERRUPT OUT.
    EP2CFG = _VALID | _TYPE0 | _TYPE1 | _BUF1;
    EP2CS = 0;

    // EP4/8 are not used.
    EP4CFG &= ~_VALID;
    EP8CFG &= ~_VALID;

    // Enable IN-BULK-NAK interrupt for EP1.
    IBNIE = _IBNI_EP1;
    NAKIE = _IBN;

    // Reset and prime EP2, and reset EP1.
    SYNCDELAY;
    FIFORESET = _NAKALL | 2;
    SYNCDELAY;
    OUTPKTEND = _SKIP | 2;
    SYNCDELAY;
    OUTPKTEND = _SKIP | 2;
    SYNCDELAY;
    FIFORESET = _NAKALL | 1;
    SYNCDELAY;
    FIFORESET = 0;

    // disables all interrupts
    EA = 0;

    // sets up all of our pins and timer.
    // we have to autopoll in lxio mode
    init_io(true);

    // Enable interrupts
    EA = 1;

    // true will reenumerate on boot.
    usb_init(true);

    uint16_t data_in_length = 0;
    bool have_lights = false;

    lxio_init();

    while (1)
    {
        io_task();

        // data coming into device
        if (!(EP2CS & _EMPTY))
        {
            data_in_length = (EP2BCH << 8) | EP2BCL;

            if (data_in_length == HID_LXIO_EPSIZE)
            {
                memcpy(&lxio_fromgame.raw_buff[0], EP2FIFOBUF, data_in_length);
                lxio_parsereport();

                // real lxio doesn't output input without the lights
                // so match that.
                have_lights = true;
            }

            EP2BCL = 0;
        }

        // data flowing out of device.
        if (pending_ep1_in && (have_lights || DEBUG_GENERIC_HID))
        {
            lxio_genreport();

            memcpy(&EP1INBUF[0], &lxio_togame.raw_buff[0], HID_LXIO_EPSIZE);

// for testing response times with evhz
#if DEBUG_GENERIC_HID
            static uint8_t counter = 0;
            EP1INBUF[0] = counter++;
#endif

            EP1INBC = HID_LXIO_EPSIZE;

            pending_ep1_in = false;
        }
    }
}
