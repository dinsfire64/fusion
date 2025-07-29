#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define KEYBOARD_VID 0x0547
#define KEYBOARD_PID 0x4B1D

#define KEYBOARD_REPORTID 0x01

// 13 keys = 5player, 5player, test/service/coin
#define HID_KEYBOARD_NUMKEYS 13

// one key per byte, plus one report byte.
#define HID_KEYBOARD_REPORT_SIZE (1 + HID_KEYBOARD_NUMKEYS)

#define HID_KEYBOARD_EPINADDR (USB_DIR_IN | 1)

// HID report for keyboard
const static __xdata uint8_t HID_Keyboard_Report[] =
    {

        0x05, 0x01, // Usage Page (Generic Desktop)
        0x09, 0x06, // Usage (Keyboard)
        0xA1, 0x01, // Collection (Application)

        0x85, KEYBOARD_REPORTID, //   Report ID (KEYBOARD_REPORTID)

        0x05, 0x07, //   Usage Page (Keyboard/Keypad)
        0x19, 0x00, //   Usage Minimum (0)
        0x29, 0xFF, //   Usage Maximum (255)

        0x15, 0x00, //   Logical Minimum (0)
        0x25, 0xFF, //   Logical Maximum (255)

        0x75, 0x08,                 //   Report Size (8 bits)
        0x95, HID_KEYBOARD_NUMKEYS, //   Report Count (HID_KEYBOARD_NUMKEYS fields) — HID_KEYBOARD_NUMKEYS-key rollover

        0x81, 0x00, //   Input (Data, Array, Absolute)

        0xC0 // End Collection
};

// scan codes
#define KEYBOARD_SCAN_Q 0x14
#define KEYBOARD_SCAN_E 0x08
#define KEYBOARD_SCAN_S 0x16
#define KEYBOARD_SCAN_Z 0x1D
#define KEYBOARD_SCAN_C 0x06

#define KEYBOARD_SCAN_F1 0x3A
#define KEYBOARD_SCAN_F2 0x3B
#define KEYBOARD_SCAN_F3 0x3C

#define KEYBOARD_SCAN_KEYPAD_1 0x59
#define KEYBOARD_SCAN_KEYPAD_3 0x5B
#define KEYBOARD_SCAN_KEYPAD_5 0x5D
#define KEYBOARD_SCAN_KEYPAD_7 0x5F
#define KEYBOARD_SCAN_KEYPAD_9 0x61

void keyboard_init(void);
bool keyboard_genreport(void);

#endif