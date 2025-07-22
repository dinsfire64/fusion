#include "lxio.h"
#include "io.h"

lxio_output_state_t volatile lxio_togame;
lxio_input_state_t volatile lxio_fromgame;

piuio_output_state_t volatile outgoing_piuio_lights;

void lxio_init(void)
{
    // lights are active high
    memset(&lxio_fromgame.raw_buff[0], 0, LXIO_PAYLOAD_SIZE);

    // output is active low.
    memset(&lxio_togame.raw_buff[0], 0xFF, LXIO_PAYLOAD_SIZE);

    outgoing_piuio_lights.raw = 0;
}

void lxio_genreport(void)
{
    // we are lucky andamiro reused all of these bytes!
    lxio_togame.cab0.raw = current_button_state.cabinet1.raw;
    lxio_togame.cab1.raw = current_button_state.cabinet2.raw;

    lxio_togame.p1_sensor0.raw = all_states[0].player1.raw;
    lxio_togame.p1_sensor1.raw = all_states[1].player1.raw;
    lxio_togame.p1_sensor2.raw = all_states[2].player1.raw;
    lxio_togame.p1_sensor3.raw = all_states[3].player1.raw;

    lxio_togame.p2_sensor0.raw = all_states[0].player2.raw;
    lxio_togame.p2_sensor1.raw = all_states[1].player2.raw;
    lxio_togame.p2_sensor2.raw = all_states[2].player2.raw;
    lxio_togame.p2_sensor3.raw = all_states[3].player2.raw;

    // still unsure what this counter is for.
    lxio_togame.counter++;
}

void lxio_parsereport(void)
{
    // again andamiro reused all of these bytes, goodie for us!
    memcpy(&outgoing_piuio_lights.buff[0], &lxio_fromgame.raw_buff[0], 4);

    // TODO: Figure out why these are reversed compared to the piuio...
    outgoing_piuio_lights.cabinet_lamps.lamp_coin_pulse = lxio_fromgame.cabinet_lamps.lamp_coin_pulse;
    outgoing_piuio_lights.cabinet_lamps.lamp_usb_en = lxio_fromgame.cabinet_lamps.lamp_usb_en;

    mux_lamp_state(&outgoing_piuio_lights, true);
}