#ifndef _PIUIO_H_
#define _PIUIO_H_

#include <stdint.h>
#include <stdbool.h>

#define PIUIO_VID 0x0547
#define PIUIO_PID 0x1002

#define PIUIO_MSG_REQ 0xAE

#define PIUIO_MSG_SIZE 8

/*
 * ------------------------------------------------
 * -------------- BITS TO CPU  ----------------
 * ------------------------------------------------
 */

// button descriptions contain pump definitions then itg definitions
// since they share the same device.
// pump never used select/menu buttons on the piuio,
// they were on a different usb device.
typedef union
{
    struct
    {
        bool btn_UL_U : 1;
        bool btn_UR_D : 1;
        bool btn_C_L : 1;
        bool btn_LL_R : 1;
        bool btn_LR_START : 1;
        bool btn_SELECT : 1;
        bool btn_MENU_LEFT : 1;
        bool btn_MENU_RIGHT : 1;
    };
    uint8_t raw;
} piuio_player_byte_t;

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
} piuio_cabinet_byte_t;

typedef union
{
    struct
    {
        piuio_player_byte_t player1;
        piuio_cabinet_byte_t cabinet1;
        piuio_player_byte_t player2;
        piuio_cabinet_byte_t cabinet2;
    };
    uint32_t raw;
    uint8_t buff[4];
} piuio_input_state_t;

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
        bool lamp_coin0_pulse : 1;
        bool lamp_coin1_usb_enable : 1;
        bool pad5 : 1;
        bool pad6 : 1;
        bool pad7 : 1;
    };
    uint8_t raw;
} piuio_cabinet_light_byte_t;

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
} piuio_neon_light_byte_t;

typedef union
{
    struct
    {
        uint8_t mux_setting : 2;
        bool lamp_ul : 1;
        bool lamp_ur : 1;
        bool lamp_c : 1;
        bool lamp_ll : 1;
        bool lamp_lr : 1;

        // this is only valid on the "p2" side for some
        // reason andamiro crossed the byte boundry...
        bool lamp_mar_ul_on_p2 : 1;
    };
    uint8_t raw;
} piuio_player_light_byte_t;

typedef union
{
    struct
    {
        piuio_player_light_byte_t p1_lamps;
        piuio_neon_light_byte_t lamp_neons;
        piuio_player_light_byte_t p2_lamps;
        piuio_cabinet_light_byte_t cabinet_lamps;
    };
    uint32_t raw;
    uint8_t buff[4];
} piuio_output_state_t;

#endif