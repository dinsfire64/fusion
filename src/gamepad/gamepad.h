#ifndef _GAMEPAD_H_
#define _GAMEPAD_H_

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define GAMEPAD_VID 0x0547
#define GAMEPAD_PID 0x1002

#define NUM_OF_BUTTONS 32

// five player, five player, neon, four marquee, coin, led
#define TOTAL_LIGHTS (5 + 5 + 1 + 4 + 1 + 1)

#define GAMEPAD_REPORTID 0x01
#define LIGHTING_REPORTID 0x02

// report id, buttons, plus two fake axis
#define HID_GAMEPAD_REPORT_SIZE (1 + (NUM_OF_BUTTONS / 8) + 2)

// number of lights plus report id.
#define HID_LIGHTS_REPORT_SIZE (1 + TOTAL_LIGHTS)

#define HID_GAMEPAD_EPINADDR (USB_DIR_IN | 1)
#define HID_LIGHTS_EPOUTADDR (USB_DIR_OUT | 1)

// where in the string buffer the ordinal strings start.
// this is index of 1 as USB "id 0" means "no string"
#define STRING_LIGHTING_START 4
#define STRING_LIGHTING_END (STRING_LIGHTING_START + TOTAL_LIGHTS)\

#define TEST_POLLING_RATE false

// HID report for lights and gamepad.
const static __xdata uint8_t HID_Gamepad_Report[] =
    {
        0x05, 0x01, // Usage Page (Generic Desktop Ctrls)
        0x09, 0x05, // Usage (Game Pad)
        0xA1, 0x01, // Collection (Application)

        0x85, GAMEPAD_REPORTID, //   Report ID (GAMEPAD_REPORTID)
        0x05, 0x09,             //   Usage Page (Button)
        0x19, 0x01,             //   Usage Minimum (0x01)
        0x29, NUM_OF_BUTTONS,   //   Usage Maximum (NUM_OF_BUTTONS)
        0x15, 0x00,             //   Logical Minimum (0)
        0x25, 0x01,             //   Logical Maximum (1)
        0x95, NUM_OF_BUTTONS,   //   Report Count (NUM_OF_BUTTONS)
        0x75, 0x01,             //   Report Size (1)
        0x81, 0x02,             //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

        0x05, 0x01, //   Usage Page (Generic Desktop Ctrls)
        0x15, 0x81, //   Logical Minimum (-127)
        0x25, 0x7F, //   Logical Maximum (127)
        0x09, 0x30, //   Usage (X)
        0x09, 0x31, //   Usage (Y)
        0x95, 0x02, //   Report Count (2)
        0x75, 0x08, //   Report Size (8)
        0x81, 0x02, //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

        0xC0, // End Collection

        0x05, 0x01,              // Usage Page (Generic Desktop Ctrls)
        0x09, 0x00,              // Usage (Undefined)
        0xA1, 0x01,              // Collection (Application)
        0x85, LIGHTING_REPORTID, //   Report ID (LIGHTING_REPORTID)
        0x95, TOTAL_LIGHTS,      //   Report Count (TOTAL_LIGHTS)
        0x75, 0x08,              //   Report Size (8)
        0x15, 0x00,              //   Logical Minimum (0)
        0x26, 0xFF, 0x00,        //   Logical Maximum (255)

        0x05, 0x0A,                  //   Usage Page (Ordinal)
        0x89, STRING_LIGHTING_START, //   String Minimum (STRING_LIGHTING_START)
        0x99, STRING_LIGHTING_END,   //   String Maximum (STRING_LIGHTING_END)
        0x19, 0x01,                  //   Usage Minimum (0x01)
        0x29, TOTAL_LIGHTS,          //   Usage Maximum (TOTAL_LIGHTS)
        0x91, 0x02,                  //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)

        0x19, 0x01, //   Usage Minimum (0x01)
        0x29, 0x01, //   Usage Maximum (0x01)
        0x81, 0x03, //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0xC0,       // End Collection

};

void gamepad_init(void);
bool gamepad_genreport(void);
void gamepad_parsereport(void);

#endif