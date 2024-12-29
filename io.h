#ifndef _IO_H_
#define _IO_H_

#include <stdint.h>
#include "piuio.h"

// TODO: Figure out why this math "makes sense"
#define TIMER0_COUNT (65536 - (48000000 / (24 * 500)))
#define TIMER1_COUNT (65536 - (48000000 / (24 * 1000)))

#define SENSOR_MUX_DELAY 100

// the original firmware delays for ~1us
#define DELAY_OUTPUT_4C 1

// the original firmware delayed for ~7us
#define DELAY_INPUT_4C 18

void init_io(void);

void push_lights(piuio_output_state_t *light_state);
piuio_input_state_t get_input_state(void);

void create_exchange(void);
void mux_lamp_state(piuio_output_state_t *light_state_from_game);

void io_task(bool autopoll);
void set_reactive_lights(void);

extern piuio_input_state_t volatile current_button_state;

#endif