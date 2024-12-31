#include <fx2regs.h>
#include <fx2debug.h>
#include <fx2ints.h>
#include <fx2usb.h>
#include <fx2lib.h>
#include <fx2usbmassstor.h>

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "io.h"
#include "piuio.h"

// allow the firmware to poll the stage pcb asynchronously
static bool autopoll = true;

// state of the IO.
piuio_input_state_t new_state_to_game;
piuio_output_state_t lamp_state_from_game;

// We perform lengthy operations in the main loop to avoid hogging the interrupt.
// This flag is used for synchronization between the main loop and the ISR;
// to allow new SETUP requests to arrive while the previous one is still being
// handled (with all data received), the flag should be reset as soon as
// the entire SETUP request is parsed.
static volatile __bit pending_setup;

// helper function not in the library yet, PR #18 against libfx2.
// https://github.com/whitequark/libfx2/pull/18
#define SETUP_EP0_OUT_BUF()   \
    do                        \
    {                         \
        SUDPTRCTL = _SDPAUTO; \
        EP0BCH = 0;           \
        EP0BCL = 0;           \
    } while (0)

usb_desc_device_c usb_device = {
    .bLength = sizeof(struct usb_desc_device),
    .bDescriptorType = USB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = USB_DEV_CLASS_PER_INTERFACE,
    .bDeviceSubClass = USB_DEV_SUBCLASS_PER_INTERFACE,
    .bDeviceProtocol = USB_DEV_PROTOCOL_PER_INTERFACE,
    .bMaxPacketSize0 = 64,
    .idVendor = PIUIO_VID,
    .idProduct = PIUIO_PID,
    .bcdDevice = 0x0000,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 0,
    .bNumConfigurations = 1,
};

usb_desc_interface_c piuio_interface = {
    .bLength = sizeof(struct usb_desc_interface),
    .bDescriptorType = USB_DESC_INTERFACE,
    .bInterfaceNumber = 0,
    .bAlternateSetting = 0,
    .bNumEndpoints = 2,
    .bInterfaceClass = 255,
    .bInterfaceSubClass = 0,
    .bInterfaceProtocol = 0,
    .iInterface = 0,
};

usb_desc_endpoint_c piuio_blkout = {
    .bLength = sizeof(struct usb_desc_endpoint),
    .bDescriptorType = USB_DESC_ENDPOINT,
    .bEndpointAddress = 2,
    .bmAttributes = USB_XFER_BULK | USB_SYNC_NONE | USB_USAGE_DATA,
    .wMaxPacketSize = 512,
    .bInterval = 0,
};

usb_desc_endpoint_c piuio_blkin = {
    .bLength = sizeof(struct usb_desc_endpoint),
    .bDescriptorType = USB_DESC_ENDPOINT,
    .bEndpointAddress = 6 | USB_DIR_IN,
    .bmAttributes = USB_XFER_BULK | USB_SYNC_NONE | USB_USAGE_DATA,
    .wMaxPacketSize = 512,
    .bInterval = 0,
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
        {.interface = &piuio_interface},
        {.endpoint = &piuio_blkout},
        {.endpoint = &piuio_blkin},
        {0},
    },
};

usb_configuration_set_c usb_configs[] = {
    &usb_config,
};

usb_ascii_string_c usb_strings[] = {
    [0] = "icedragon.io",
    [1] = "fusion-piuio",
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

bool handle_piuio_msg(void)
{
    __xdata struct usb_req_setup *req = (__xdata struct usb_req_setup *)SETUPDAT;

    // check for "magic byte" of PIUIO_MSG_REQ
    // original only responds/uses length of 8, but only 4 is used
    // allow anywhere between 4-8 to be polled, and we will respond with that length.
    if (req->bRequest == PIUIO_MSG_REQ &&
        req->wLength >= 4 &&
        req->wLength <= PIUIO_MSG_SIZE)
    {
        if ((req->bmRequestType & USB_DIR_MASK) == USB_DIR_OUT)
        {
            // getting lighting data from game

            pending_setup = false;

            SETUP_EP0_OUT_BUF();
            while (EP0CS & _BUSY)
                ;

            memcpy(&lamp_state_from_game.buff[0], &EP0BUF[0], 4);

            if (autopoll)
            {
                mux_lamp_state(&lamp_state_from_game);
            }
            else
            {
                push_lights(&lamp_state_from_game);

                // let the sensor mux settle.
                delay_us(SENSOR_MUX_DELAY);
            }

            // safe to overwrite EP0BUF since I have pulled the data out and processed it.
            ACK_EP0();

            return true;
        }
        else
        {
            // pushing button state to game

            while (EP0CS & _BUSY)
                ;

            if (autopoll)
            {
                memcpy(&new_state_to_game.buff[0], &current_button_state.buff[0], 4);
            }
            else
            {
                new_state_to_game = get_input_state();
            }

            // buttons are active low, so set high.
            memset(&EP0BUF[0], 0xFF, (uint8_t)req->wLength);
            memcpy(&EP0BUF[0], &new_state_to_game.buff[0], 4);

            // these are always zero for some reason?
            EP0BUF[6] = 0;
            EP0BUF[7] = 0;

            SETUP_EP0_BUF((uint8_t)req->wLength);

            pending_setup = false;

            return true;
        }
    }

    pending_setup = false;
    return false;
}

// callback for control transfer requests.
void handle_usb_setup(__xdata struct usb_req_setup *req)
{
    req;

    if (pending_setup)
    {
        // we are busy, please come back later.
        // kpump didn't like this despite it being legal to do...
        STALL_EP0();
    }
    else
    {
        pending_setup = true;
    }
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

    // EP2 is configured as 512-byte double buffed BULK OUT.
    EP2CFG = _VALID | _TYPE1 | _BUF1;
    EP2CS = 0;
    // EP6 is configured as 512-byte double buffed BULK IN.
    EP6CFG = _VALID | _DIR | _TYPE1 | _BUF1;
    EP6CS = 0;
    // EP4/8 are not used.
    EP4CFG &= ~_VALID;
    EP8CFG &= ~_VALID;

    // Enable IN-BULK-NAK interrupt for EP6.
    IBNIE = _IBNI_EP6;
    NAKIE = _IBN;

    // Reset and prime EP2, and reset EP6.
    SYNCDELAY;
    FIFORESET = _NAKALL | 2;
    SYNCDELAY;
    OUTPKTEND = _SKIP | 2;
    SYNCDELAY;
    OUTPKTEND = _SKIP | 2;
    SYNCDELAY;
    FIFORESET = _NAKALL | 6;
    SYNCDELAY;
    FIFORESET = 0;

    // disables all interrupts
    EA = 0;

    // sets up all of our pins and timer.
    init_io(autopoll);

    // Enable interrupts
    EA = 1;

    // true will reenumerate on boot.
    usb_init(true);

    // turn on the LED to indicate firmware has loaded.
    // if you have something running that will autoconnect to the piuio (like the linux kernel module)
    // this will not stay lit for long, since a full lighting state will be written.
    lamp_state_from_game.raw = 0;
    lamp_state_from_game.lamp_neons.lamp_led = true;
    mux_lamp_state(&lamp_state_from_game);
    push_lights(&lamp_state_from_game);

    while (1)
    {
        if (pending_setup)
        {
            handle_piuio_msg();
        }

        io_task();
    }
}
