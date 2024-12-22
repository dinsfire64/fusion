#include "piuio.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <usb.h>
#include <stdbool.h>

int main(int argc, char *argv[])
{
    struct usb_bus *bus;

    struct usb_device *piuio_dev = NULL;
    usb_dev_handle *piuio_udev;

    uint8_t iDataPIUIO[PIUIO_MSG_SIZE];

    memset(iDataPIUIO, 0, PIUIO_MSG_SIZE);

    usb_init();

    usb_find_busses();
    usb_find_devices();

    for (bus = usb_busses; bus; bus = bus->next)
    {
        struct usb_device *dev;

        for (dev = bus->devices; dev; dev = dev->next)
        {
            if (dev->descriptor.idVendor == PIUIO_VID &&
                dev->descriptor.idProduct == PIUIO_PID)
            {
                piuio_dev = dev;
            }
        }
    }

    if (piuio_dev)
    {
        piuio_udev = usb_open(piuio_dev);

        if (piuio_udev)
        {
            usb_set_configuration(piuio_udev, 0);
            usb_claim_interface(piuio_udev, 0);
            printf("piuio opened\n");

            uint8_t bit_loc = 0;
            uint8_t byte_loc = 0;
            uint8_t sensor_num = 0;
            bool toggle = false;

            printf("starting...\n");

            int i;
            while (1)
            {
                bit_loc++;

                if (bit_loc >= 8)
                {
                    bit_loc = 0;
                    byte_loc++;

                    if (byte_loc >= 8)
                    {
                        byte_loc = 0;
                    }
                }

                // printf("send: %d:%d\n", byte_loc, bit_loc);

                memset(iDataPIUIO, 0, PIUIO_MSG_SIZE);
                iDataPIUIO[byte_loc] = (1 << bit_loc);

                // iDataPIUIO[0] = iDataPIUIO[0] & 0x03;
                // iDataPIUIO[0] |= sensor_num & 0x03;
                // iDataPIUIO[2] = iDataPIUIO[2] & 0x03;
                // iDataPIUIO[2] |= sensor_num & 0x03;
                // sensor_num++;

                printf("send: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n", iDataPIUIO[0],
                       iDataPIUIO[1], iDataPIUIO[2], iDataPIUIO[3],
                       iDataPIUIO[4], iDataPIUIO[5], iDataPIUIO[6], iDataPIUIO[7]);

                // USB_SEND
                usb_control_msg(piuio_udev, PIUIO_REQTYPE_WRITELIGHT, PIUIO_CMD_MSG, 0, 0, iDataPIUIO,
                                PIUIO_MSG_SIZE, 10000);

                // sleep(1);

                // USB_GET
                usb_control_msg(piuio_udev, PIUIO_REQTYPE_GETINPUT, PIUIO_CMD_MSG, 0, 0, iDataPIUIO,
                                PIUIO_MSG_SIZE, 10000);

                printf("recv: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n", iDataPIUIO[0],
                       iDataPIUIO[1], iDataPIUIO[2], iDataPIUIO[3],
                       iDataPIUIO[4], iDataPIUIO[5], iDataPIUIO[6], iDataPIUIO[7]);

                // usleep(0.001 * 1000 * 1000);

                // USB_SEND
                // usb_control_msg(piuio_udev, 0x40, PIUIO_CMD_MSG, 0, 0, iDataPIUIO,
                // PIUIO_MSG_SIZE, 2000);
            }
        }
        else
        {
            printf("Could not open piuio\n");
        }
    }
    else
    {
        printf("Could not find piuio\n");
    }

    return 0;
}
