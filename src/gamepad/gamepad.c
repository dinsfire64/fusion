#include "gamepad.h"
#include "io.h"

volatile uint8_t gamepad_tocpu[HID_GAMEPAD_REPORT_SIZE];
volatile uint8_t gamepad_fromcpu[HID_LIGHTS_REPORT_SIZE];

piuio_output_state_t volatile outgoing_piuio_lights;

void gamepad_init(void)
{
    // lights are active high
    memset(&gamepad_fromcpu[0], 0, HID_LIGHTS_REPORT_SIZE);

    // output is active high.
    memset(&gamepad_tocpu[0], 0, HID_GAMEPAD_REPORT_SIZE);

    outgoing_piuio_lights.raw = 0;
}

void gamepad_genreport(void)
{
    memset(&gamepad_tocpu[0], 0, HID_GAMEPAD_REPORT_SIZE);

    gamepad_tocpu[0] = GAMEPAD_REPORTID;

    // piuio is active low, hid is active high, so flip.
    gamepad_tocpu[1] = ~current_button_state.player1.raw;
    gamepad_tocpu[2] = ~current_button_state.cabinet1.raw;
    gamepad_tocpu[3] = ~current_button_state.player2.raw;
    gamepad_tocpu[4] = ~current_button_state.cabinet2.raw;

    // fake axis centers at zero, so no need to change.

    // for testing speed with evhz
    // static uint8_t counter = 0;
    // gamepad_tocpu[1] = counter++;
}

void gamepad_parsereport(void)
{
    if (gamepad_fromcpu[0] == LIGHTING_REPORTID)
    {
        outgoing_piuio_lights.p1_lamps.lamp_ul = gamepad_fromcpu[1] > 0;
        outgoing_piuio_lights.p1_lamps.lamp_ur = gamepad_fromcpu[2] > 0;
        outgoing_piuio_lights.p1_lamps.lamp_cn = gamepad_fromcpu[3] > 0;
        outgoing_piuio_lights.p1_lamps.lamp_ll = gamepad_fromcpu[4] > 0;
        outgoing_piuio_lights.p1_lamps.lamp_lr = gamepad_fromcpu[5] > 0;

        outgoing_piuio_lights.p2_lamps.lamp_ul = gamepad_fromcpu[6] > 0;
        outgoing_piuio_lights.p2_lamps.lamp_ur = gamepad_fromcpu[7] > 0;
        outgoing_piuio_lights.p2_lamps.lamp_cn = gamepad_fromcpu[8] > 0;
        outgoing_piuio_lights.p2_lamps.lamp_ll = gamepad_fromcpu[9] > 0;
        outgoing_piuio_lights.p2_lamps.lamp_lr = gamepad_fromcpu[10] > 0;

        outgoing_piuio_lights.lamp_neons.lamp_neon = gamepad_fromcpu[11] > 0;

        outgoing_piuio_lights.p2_lamps.lamp_mar_ul_on_p2 = gamepad_fromcpu[12] > 0;
        outgoing_piuio_lights.cabinet_lamps.lamp_maq_ur = gamepad_fromcpu[13] > 0;
        outgoing_piuio_lights.cabinet_lamps.lamp_maq_ll = gamepad_fromcpu[14] > 0;
        outgoing_piuio_lights.cabinet_lamps.lamp_maq_lr = gamepad_fromcpu[15] > 0;

        outgoing_piuio_lights.cabinet_lamps.lamp_coin_pulse = gamepad_fromcpu[16] > 0;
        outgoing_piuio_lights.cabinet_lamps.lamp_usb_en = gamepad_fromcpu[17] > 0;

        outgoing_piuio_lights.lamp_neons.lamp_led = gamepad_fromcpu[18] > 0;

        mux_lamp_state(&outgoing_piuio_lights);
    }
}