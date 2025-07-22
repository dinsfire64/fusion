#include "gamepad.h"
#include "io.h"

#include <fx2regs.h>

// for changing endpoints in the future.
#define EPIN_BUFFER EP1INBUF
#define EPOUT_BUFFER EP1OUTBUF

// keeps track of the last state sent to the CPU.
uint8_t prev_gamepad_tocpu[HID_GAMEPAD_REPORT_SIZE];

piuio_output_state_t outgoing_piuio_lights;

void gamepad_init(void)
{
    // active high.
    memset(&prev_gamepad_tocpu[0], 0, HID_GAMEPAD_REPORT_SIZE);

    // all lights off on start.
    outgoing_piuio_lights.raw = 0;
}

bool gamepad_genreport(void)
{
    bool changed = false;

    memset(&EPIN_BUFFER[0], 0, HID_GAMEPAD_REPORT_SIZE);

    EPIN_BUFFER[0] = GAMEPAD_REPORTID;

    // piuio is active low, hid is active high, so flip.
    EPIN_BUFFER[1] = ~current_button_state.player1.raw;
    EPIN_BUFFER[2] = ~current_button_state.cabinet1.raw;
    EPIN_BUFFER[3] = ~current_button_state.player2.raw;
    EPIN_BUFFER[4] = ~current_button_state.cabinet2.raw;

    // fake axis centers at zero, so no need to change.

#if TEST_POLLING_RATE
    // for testing speed with evhz
    static uint8_t counter = 0;
    EPIN_BUFFER[1] = counter++;
#endif

    // save time with own compare by checking only 1-4
    // (report id and axis are static.)
    for (int i = 1; i < 5; i++)
    {
        if (prev_gamepad_tocpu[i] != EPIN_BUFFER[i])
        {
            changed = true;
            break;
        }
    }

    // only memcpy the actual report if changed.
    if (changed)
    {
        memcpy(&prev_gamepad_tocpu[1], &EPIN_BUFFER[1], 4);
    }

    return changed;
}

void gamepad_parsereport(void)
{
    if (EPOUT_BUFFER[0] == LIGHTING_REPORTID)
    {
        outgoing_piuio_lights.p1_lamps.lamp_ul = EPOUT_BUFFER[1] > 0;
        outgoing_piuio_lights.p1_lamps.lamp_ur = EPOUT_BUFFER[2] > 0;
        outgoing_piuio_lights.p1_lamps.lamp_cn = EPOUT_BUFFER[3] > 0;
        outgoing_piuio_lights.p1_lamps.lamp_ll = EPOUT_BUFFER[4] > 0;
        outgoing_piuio_lights.p1_lamps.lamp_lr = EPOUT_BUFFER[5] > 0;

        outgoing_piuio_lights.p2_lamps.lamp_ul = EPOUT_BUFFER[6] > 0;
        outgoing_piuio_lights.p2_lamps.lamp_ur = EPOUT_BUFFER[7] > 0;
        outgoing_piuio_lights.p2_lamps.lamp_cn = EPOUT_BUFFER[8] > 0;
        outgoing_piuio_lights.p2_lamps.lamp_ll = EPOUT_BUFFER[9] > 0;
        outgoing_piuio_lights.p2_lamps.lamp_lr = EPOUT_BUFFER[10] > 0;

        outgoing_piuio_lights.lamp_neons.lamp_neon = EPOUT_BUFFER[11] > 0;

        outgoing_piuio_lights.p2_lamps.lamp_mar_ul_on_p2 = EPOUT_BUFFER[12] > 0;
        outgoing_piuio_lights.cabinet_lamps.lamp_maq_ur = EPOUT_BUFFER[13] > 0;
        outgoing_piuio_lights.cabinet_lamps.lamp_maq_ll = EPOUT_BUFFER[14] > 0;
        outgoing_piuio_lights.cabinet_lamps.lamp_maq_lr = EPOUT_BUFFER[15] > 0;

        outgoing_piuio_lights.cabinet_lamps.lamp_coin_pulse = EPOUT_BUFFER[16] > 0;
        outgoing_piuio_lights.cabinet_lamps.lamp_usb_en = EPOUT_BUFFER[17] > 0;

        outgoing_piuio_lights.lamp_neons.lamp_led = EPOUT_BUFFER[18] > 0;

        mux_lamp_state(&outgoing_piuio_lights, true);
    }
}