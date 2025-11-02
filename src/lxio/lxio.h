#ifndef _LXIO_H_
#define _LXIO_H_

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define LXIO_VID 0x0D2F
#define LXIO_PID_V1 0x1020
#define LXIO_PID_V2 0x1040

#define HID_LXIO_EPINADDR (USB_DIR_IN | 1)
#define HID_LXIO_EPOUTADDR (USB_DIR_OUT | 2)

#define HID_LXIO_EPSIZE 16
#define LXIO_PAYLOAD_SIZE 16

// generic gamepad hid for testing response times with standard programs like evhz
#define DEBUG_GENERIC_HID false

// HID report

static const __code uint8_t HID_LXIO_Report[] =
    {
#if !DEBUG_GENERIC_HID
        0x06, 0x00, 0xFF, // Usage Page (Vendor Defined 0xFF00)
        0x09, 0x01,       // Usage (0x01)
        0xA1, 0x01,       // Collection (Application)
        0x09, 0x02,       //   Usage (0x02)
        0x15, 0x00,       //   Logical Minimum (0)
        0x25, 0xFF,       //   Logical Maximum (-1)
        0x75, 0x08,       //   Report Size (8)
        0x95, 0x10,       //   Report Count (16)
        0x81, 0x02,       //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x09, 0x03,       //   Usage (0x03)
        0x15, 0x00,       //   Logical Minimum (0)
        0x25, 0xFF,       //   Logical Maximum (-1)
        0x75, 0x08,       //   Report Size (8)
        0x95, 0x10,       //   Report Count (16)
        0x91, 0x02,       //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0xC0,             // End Collection
#else
        0x05, 0x01, // Usage Page (Generic Desktop Ctrls)
        0x09, 0x05, // Usage (Game Pad)
        0xA1, 0x01, // Collection (Application)
        0x05, 0x09, //   Usage Page (Button)
        0x19, 0x01, //   Usage Minimum (0x01)
        0x29, 0x80, //   Usage Maximum (0x80)
        0x15, 0x00, //   Logical Minimum (0)
        0x25, 0x01, //   Logical Maximum (1)
        0x95, 0x80, //   Report Count (-128)
        0x75, 0x01, //   Report Size (1)
        0x81, 0x02, //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0xC0,       // End Collection
#endif
};

/*
 * ------------------------------------------------
 * -------------- BITS TO CPU  ----------------
 * ------------------------------------------------
 */

typedef union
{
    struct
    {
        bool btn_UL_U : 1;
        bool btn_UR_D : 1;
        bool btn_CN_L : 1;
        bool btn_LL_R : 1;
        bool btn_LR_START : 1;
        bool btn_SELECT : 1;
        bool btn_MENU_LEFT : 1;
        bool btn_MENU_RIGHT : 1;
    };
    uint8_t raw;
} lxio_player_byte_t;

typedef union
{
    struct
    {
        bool pad0 : 1;
        bool btn_TEST : 1;
        bool btn_COIN : 1;
        bool pad3 : 1;
        bool pad4 : 1;
        bool pad5 : 1;
        bool btn_SERVICE : 1;
        bool btn_CLR : 1;
    };
    uint8_t raw;
} lxio_cabinet_byte_t;

typedef union
{
    struct
    {
        bool btn_MENU_UL : 1;
        bool btn_MENU_UR : 1;
        bool btn_MENU_CN : 1;
        bool btn_MENU_LL : 1;
        bool btn_MENU_LR : 1;
        bool pad5 : 1;
        bool pad6 : 1;
        bool pad7 : 1;
    };
    uint8_t raw;
} lxio_menu_byte_t;

typedef union
{
    struct
    {
        lxio_player_byte_t p1_sensor0;
        lxio_player_byte_t p1_sensor1;
        lxio_player_byte_t p1_sensor2;
        lxio_player_byte_t p1_sensor3;

        lxio_player_byte_t p2_sensor0;
        lxio_player_byte_t p2_sensor1;
        lxio_player_byte_t p2_sensor2;
        lxio_player_byte_t p2_sensor3;

        lxio_cabinet_byte_t cab0;
        lxio_cabinet_byte_t cab1;

        lxio_menu_byte_t p1_menu;
        lxio_menu_byte_t p2_menu;

        uint8_t byte12;
        uint8_t byte13;
        // TODO: Figure out what this counter does. Increments on every packet?
        uint16_t counter;
    };
    uint8_t raw_buff[LXIO_PAYLOAD_SIZE];
} lxio_output_state_t;

/*
 * ----------------------------------------------
 * -------------- BITS FROM CPU  ----------------
 * ----------------------------------------------
 */

typedef union
{
    struct
    {
        bool lamp_maq_lr : 1;
        bool lamp_maq_ll : 1;
        bool lamp_maq_ur : 1;
        // NOTE: reversed from piuio for some reason?
        bool lamp_coin_pulse : 1;
        bool lamp_usb_en : 1;
        bool lamp_alwayson0 : 1;
        bool lamp_alwayson1 : 1;
        bool lamp_alwaysoff0 : 1;
    };
    uint8_t raw;
} lxio_cabinet_light_byte_t;

typedef union
{
    struct
    {
        bool pad0 : 1;
        bool pad1 : 1;
        bool lamp_neon : 1;
        bool pad3 : 1;
        bool pad4 : 1;
        bool pad5 : 1;
        bool lamp_led : 1;
        bool pad7 : 1;
    };
    uint8_t raw;
} lxio_neon_light_byte_t;

typedef union
{
    struct
    {
        uint8_t mux_setting : 2;
        bool lamp_ul : 1;
        bool lamp_ur : 1;
        bool lamp_cn : 1;
        bool lamp_ll : 1;
        bool lamp_lr : 1;

        // this is only valid on the "p2" side for some
        // reason andamiro crossed the byte boundry...
        bool lamp_mar_ul_on_p2 : 1;
    };
    uint8_t raw;
} lxio_player_light_byte_t;

typedef union
{
    struct
    {
        bool menu_ul : 1;
        bool menu_ur : 1;
        bool menu_cn : 1;
        bool menu_ll : 1;
        bool menu_lr : 1;
        bool pad5 : 1;
        bool pad6 : 1;
        bool pad7 : 1;
    };
    uint8_t raw;
} lxio_menu_light_byte_t;

typedef union
{
    struct
    {
        lxio_player_light_byte_t p1_lamps;
        lxio_neon_light_byte_t lamp_neons;
        lxio_player_light_byte_t p2_lamps;
        lxio_cabinet_light_byte_t cabinet_lamps;
        lxio_menu_light_byte_t p1_menu_lamps;
        lxio_menu_light_byte_t p2_menu_lamps;
        uint8_t byte6;
        uint8_t byte7;

        uint8_t byte8;
        uint8_t byte9;
        uint8_t byte10;
        uint8_t byte11;
        uint8_t byte12;
        uint8_t byte13;
        uint8_t byte14;
        uint8_t byte15;
    };
    uint8_t raw_buff[LXIO_PAYLOAD_SIZE];
} lxio_input_state_t;

extern volatile lxio_output_state_t lxio_togame;
extern volatile lxio_input_state_t lxio_fromgame;

void lxio_init(void);
void lxio_genreport(void);
void lxio_parsereport(void);

#endif