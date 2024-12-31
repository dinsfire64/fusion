#ifndef _LXIO_H
#define _LXIO_H

#define LXIO_VID 0x0D2F
#define LXIO_PID_V1 0x1020
#define LXIO_PID_V2 0x1040

// NOTE: libusb seems to only want the index,
// despite the full address containing the MSB being the direction.
#define LXIO_ENDPOINT_INPUT 1
#define LXIO_ENDPOINT_OUT 2

#define LXIO_MSG_SIZE 16

#endif
