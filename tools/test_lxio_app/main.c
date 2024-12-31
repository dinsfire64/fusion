#include "lxio.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <usb.h>
#include <stdbool.h>

int main(int argc, char *argv[])
{
    struct usb_bus *bus;

    struct usb_device *usb_dev = NULL;
    usb_dev_handle *usb_udev;

    uint8_t iDataLXIO[LXIO_MSG_SIZE];

    // lights are active high.
    memset(iDataLXIO, 0, LXIO_MSG_SIZE);

    usb_init();

    usb_find_busses();
    usb_find_devices();

    for (bus = usb_busses; bus; bus = bus->next)
    {
        struct usb_device *dev;

        for (dev = bus->devices; dev; dev = dev->next)
        {
            if (dev->descriptor.idVendor == LXIO_VID &&
                dev->descriptor.idProduct == LXIO_PID_V1)
            {
                usb_dev = dev;
            }
        }
    }

    if (usb_dev)
    {
        usb_udev = usb_open(usb_dev);

        if (usb_udev)
        {
            int result = -1;

            for (int i = 0; i < usb_dev->config->bNumInterfaces; i++)
            {
                usb_detach_kernel_driver_np(usb_udev, i);
                usb_claim_interface(usb_udev, i);
                usb_set_configuration(usb_udev, i);
            }

            printf("lxio opened\n");

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

                    if (byte_loc >= LXIO_MSG_SIZE)
                    {
                        byte_loc = 0;
                    }
                }

                // printf("send: %d:%d\n", byte_loc, bit_loc);

                memset(iDataLXIO, 0, LXIO_MSG_SIZE);
                iDataLXIO[byte_loc] = (1 << bit_loc);

                printf("send: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n", iDataLXIO[0],
                       iDataLXIO[1], iDataLXIO[2], iDataLXIO[3],
                       iDataLXIO[4], iDataLXIO[5], iDataLXIO[6],
                       iDataLXIO[7], iDataLXIO[8], iDataLXIO[9],
                       iDataLXIO[10], iDataLXIO[11], iDataLXIO[12],
                       iDataLXIO[13], iDataLXIO[14], iDataLXIO[15]);

                // device wants to be written to then read.
                result = usb_interrupt_write(usb_udev, LXIO_ENDPOINT_OUT, iDataLXIO, sizeof(iDataLXIO), 10000);

                if (result < 0)
                {
                    printf("%d: %s\n", result, usb_strerror());
                }

                result = usb_interrupt_read(usb_udev, LXIO_ENDPOINT_INPUT, iDataLXIO, sizeof(iDataLXIO), 10000);

                if (result < 0)
                {
                    printf("%d: %s\n", result, usb_strerror());
                }

                printf("recv: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n", iDataLXIO[0],
                       iDataLXIO[1], iDataLXIO[2], iDataLXIO[3],
                       iDataLXIO[4], iDataLXIO[5], iDataLXIO[6],
                       iDataLXIO[7], iDataLXIO[8], iDataLXIO[9],
                       iDataLXIO[10], iDataLXIO[11], iDataLXIO[12],
                       iDataLXIO[13], iDataLXIO[14], iDataLXIO[15]);

                usleep(0.001 * 1000 * 1000);
            }
        }
        else
        {
            printf("Could not open lxio\n");
        }
    }
    else
    {
        printf("Could not find lxio\n");
    }

    return 0;
}
