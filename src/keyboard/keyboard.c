#include "keyboard.h"
#include "io.h"

#include <fx2regs.h>

// for changing endpoints in the future.
#define EPIN_BUFFER EP1INBUF

// keeps track of the last state sent to the CPU.
uint8_t prev_keyboard_tocpu[HID_KEYBOARD_REPORT_SIZE];

piuio_output_state_t outgoing_piuio_lights;

void keyboard_init(void)
{
    // active high.
    memset(&prev_keyboard_tocpu[0], 0, HID_KEYBOARD_REPORT_SIZE);

    // all lights off on start.
    outgoing_piuio_lights.raw = 0;

    outgoing_piuio_lights.lamp_neons.lamp_led = true;

    // force the usb enable to be on all the time in this mode.
    outgoing_piuio_lights.cabinet_lamps.lamp_usb_en = true;
    mux_lamp_state(&outgoing_piuio_lights, false);
}

bool keyboard_genreport(void)
{
    bool changed = false;

    memset(&EPIN_BUFFER[0], 0, HID_KEYBOARD_REPORT_SIZE);

    EPIN_BUFFER[0] = KEYBOARD_REPORTID;

    // active low, so true is "off"

    // pump P1 qeszc
    EPIN_BUFFER[1] = current_button_state.player1.btn_UL_U ? 0x00 : KEYBOARD_SCAN_Q;
    EPIN_BUFFER[2] = current_button_state.player1.btn_UR_D ? 0x00 : KEYBOARD_SCAN_E;
    EPIN_BUFFER[3] = current_button_state.player1.btn_CN_L ? 0x00 : KEYBOARD_SCAN_S;
    EPIN_BUFFER[4] = current_button_state.player1.btn_LL_R ? 0x00 : KEYBOARD_SCAN_Z;
    EPIN_BUFFER[5] = current_button_state.player1.btn_LR_START ? 0x00 : KEYBOARD_SCAN_C;

    // pump P2 79513
    EPIN_BUFFER[6] = current_button_state.player2.btn_UL_U ? 0x00 : KEYBOARD_SCAN_KEYPAD_7;
    EPIN_BUFFER[7] = current_button_state.player2.btn_UR_D ? 0x00 : KEYBOARD_SCAN_KEYPAD_9;
    EPIN_BUFFER[8] = current_button_state.player2.btn_CN_L ? 0x00 : KEYBOARD_SCAN_KEYPAD_5;
    EPIN_BUFFER[9] = current_button_state.player2.btn_LL_R ? 0x00 : KEYBOARD_SCAN_KEYPAD_1;
    EPIN_BUFFER[10] = current_button_state.player2.btn_LR_START ? 0x00 : KEYBOARD_SCAN_KEYPAD_3;

    // pump test/service/coin F1/2/3
    EPIN_BUFFER[11] = current_button_state.cabinet1.btn_SERVICE ? 0x00 : KEYBOARD_SCAN_F1;
    EPIN_BUFFER[12] = current_button_state.cabinet1.btn_TEST ? 0x00 : KEYBOARD_SCAN_F2;
    EPIN_BUFFER[13] = current_button_state.cabinet1.btn_COIN ? 0x00 : KEYBOARD_SCAN_F3;

    // only memcpy the actual report if changed.
    if (memcmp(prev_keyboard_tocpu, EPIN_BUFFER, HID_KEYBOARD_REPORT_SIZE) != 0)
    {
        memcpy(prev_keyboard_tocpu, EPIN_BUFFER, HID_KEYBOARD_REPORT_SIZE);
        changed = true;
    }

    return changed;
}