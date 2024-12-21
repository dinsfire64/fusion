#ifndef _IO_H_
#define _IO_H_

#include <stdint.h>
#include "piuio.h"

// TODO: Figure out why this math "makes sense"
#define TIMER0_COUNT (65536 - (48000000 / (24 * 500)))

void init_io(void);

void push_lights(piuio_output_state_t *light_state);
piuio_input_state_t get_input_state(void);

#endif