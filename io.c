#include "io.h"

#include <fx2regs.h>
#include <fx2debug.h>
#include <stdio.h>
#include <fx2ints.h>

// Register an interrupt handler for TIMER0 overflow
void isr_TF0(void) __interrupt(_INT_TF0)
{
    // toggle the reset line for the output latches.
    PD7 = !PD7;

    // set the timer to fire again.
    TR0 = 0;
    TL0 = TL0 + (TIMER0_COUNT & 0x00FF);
    TH0 = TH0 + (TIMER0_COUNT >> 8);
    TR0 = 1;
}

void init_io(void)
{
    // configure the latching / clock pins
    // all outputs.
    IOD = 0xFF;

    // PD0, PD1 for input clock
    // PD4, PD5 for output clock
    // PD7 for reset circuit (it's toggled by the timer interrupt)
    // =0xB3
    OED = 0xB3;

    // init values, disable output, disable input.
    PD4 = 0;
    PD5 = 0;

    PD0 = 1;
    PD1 = 1;

    // startup timer, taken from cypress example.
    TR0 = 0;                       // stops Timer 0
    CKCON = 0x03;                  // Timer 0 using CLKOUT/12
    TMOD &= ~0x0F;                 // clear Timer 0 mode bits
    TMOD |= 0x01;                  // setup Timer 0 as a 16-bit timer
    TL0 = (TIMER0_COUNT & 0x00FF); // loads the timer counts
    TH0 = (TIMER0_COUNT >> 8);
    PT0 = 0; // sets the Timer 0 interrupt to low priority
    ET0 = 1; // enables Timer 0 interrupt
    TR0 = 1; // starts Timer 0
}

void push_lights(piuio_output_state_t *light_state)
{
    // set to output
    OEA = 0xFF;
    OEB = 0xFF;

    // push first half of state.
    IOA = light_state->buff[0];
    IOB = light_state->buff[1];

    // latch high
    PD4 = 1;
    PD4 = 0;

    // push last half of state.
    IOA = light_state->buff[2];
    IOB = light_state->buff[3];

    // latch high
    PD5 = 1;
    PD5 = 0;

    // original decomp does this, so repeating.
    IOA = 0xFF;
    IOB = 0xFF;

    IOA = 0x00;
    IOB = 0x00;

    // return to input.
    OEA = 0x00;
    OEB = 0x00;
}

piuio_input_state_t get_input_state(void)
{
    piuio_input_state_t new_state;

    // original decomp did instructions twice, so we are doing the same.
    // perhaps a way to slow down/ensure the 74 logic meets setup/hold?

    // turn into input.
    OEA = 0x00;
    OEB = 0x00;
    OEA = 0x00;
    OEB = 0x00;

    // latch, then sample.
    PD1 = 0;
    PD1 = 0;

    new_state.buff[3] = IOB;
    new_state.buff[2] = IOA;

    PD1 = 1;
    PD1 = 1;

    // latch, then sample.
    PD0 = 0;
    PD0 = 0;

    new_state.buff[1] = IOB;
    new_state.buff[0] = IOA;

    PD0 = 1;
    PD0 = 1;

    // original firmware also toggles PD2, but it's not used on the final board
    // so skip.

    return new_state;
}