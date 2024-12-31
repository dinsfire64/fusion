#include "io.h"

#include <fx2regs.h>
#include <fx2debug.h>
#include <stdio.h>
#include <fx2ints.h>
#include <string.h>

piuio_input_state_t volatile current_button_state;
piuio_output_state_t volatile current_lamp_state;

piuio_output_state_t volatile current_game_lamp_state;

piuio_input_state_t volatile all_states[PIUIO_NUM_SENSORS];
uint8_t current_sensor_num = 0;

static volatile bool preform_poll = false;
bool autopoll_enabled = false;

void set_sensor_mux(void)
{
    // always override in autopoll mode, we are the only ones allowed to touch this.
    current_lamp_state.p1_lamps.mux_setting = current_sensor_num;
    current_lamp_state.p2_lamps.mux_setting = current_sensor_num;
}

void set_reactive_lights(void)
{
    // TODO ITG swapped p1/p2 lights, so offer a swap function/setting.
    current_lamp_state.p1_lamps.lamp_ul = !current_button_state.player1.btn_UL_U | current_game_lamp_state.p1_lamps.lamp_ul;
    current_lamp_state.p1_lamps.lamp_ur = !current_button_state.player1.btn_UR_D | current_game_lamp_state.p1_lamps.lamp_ur;
    current_lamp_state.p1_lamps.lamp_cn = !current_button_state.player1.btn_CN_L | current_game_lamp_state.p1_lamps.lamp_cn;
    current_lamp_state.p1_lamps.lamp_ll = !current_button_state.player1.btn_LL_R | current_game_lamp_state.p1_lamps.lamp_ll;
    current_lamp_state.p1_lamps.lamp_lr = !current_button_state.player1.btn_LR_START | current_game_lamp_state.p1_lamps.lamp_lr;

    current_lamp_state.p2_lamps.lamp_ul = !current_button_state.player2.btn_UL_U | current_game_lamp_state.p2_lamps.lamp_ul;
    current_lamp_state.p2_lamps.lamp_ur = !current_button_state.player2.btn_UR_D | current_game_lamp_state.p2_lamps.lamp_ur;
    current_lamp_state.p2_lamps.lamp_cn = !current_button_state.player2.btn_CN_L | current_game_lamp_state.p2_lamps.lamp_cn;
    current_lamp_state.p2_lamps.lamp_ll = !current_button_state.player2.btn_LL_R | current_game_lamp_state.p2_lamps.lamp_ll;
    current_lamp_state.p2_lamps.lamp_lr = !current_button_state.player2.btn_LR_START | current_game_lamp_state.p2_lamps.lamp_lr;
}

void create_exchange(void)
{
    // set the mux state
    set_sensor_mux();

    // pretty lights :)
    set_reactive_lights();

    // output the new mux state
    push_lights(&current_lamp_state);

    // slight delay before pulling the answer, for setup/hold times.
    delay_us(SENSOR_MUX_DELAY);

    // then input the result.
    all_states[current_sensor_num] = get_input_state();

    // take the first pressed sensor as gospel.
    // in rhythm games time pressed is more important than time released.
    // input coming in is active high, so logic AND together.
    current_button_state.raw &= all_states[current_sensor_num].raw;

    if (current_sensor_num >= (PIUIO_NUM_SENSORS - 1))
    {
        // once we have all four sensors, we need to check
        // for a release by ANDing (active low) then all together.
        piuio_input_state_t temp_btn_state;
        temp_btn_state.raw = UINT32_MAX;

        for (int i = 0; i < PIUIO_NUM_SENSORS; i++)
        {
            temp_btn_state.raw &= all_states[i].raw;
        }

        current_button_state.raw = temp_btn_state.raw;
        current_sensor_num = 0;
    }
    else
    {
        current_sensor_num++;
    }
}

// timer used to keep the reset circuitry high for the latching outputs.
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

// timer used to kick off a sensor polling event.
void isr_TF1(void) __interrupt(_INT_TF0)
{
    preform_poll = true;

    // set the timer to fire again.
    TR1 = 0;
    TL1 = TL1 + (TIMER1_COUNT & 0x00FF);
    TH1 = TH1 + (TIMER1_COUNT >> 8);
    TR1 = 1;
}

void init_io(bool en_autopoll)
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

    if (en_autopoll)
    {
        TR1 = 0;                       // stops Timer 1
        TMOD &= ~0xF0;                 // clear Timer 1 mode bits
        TMOD |= 0x10;                  // setup Timer 11 as a 16-bit timer
        TL1 = (TIMER1_COUNT & 0x00FF); // loads the timer counts
        TH1 = (TIMER1_COUNT >> 8);
        PT1 = 0; // sets the Timer 1 interrupt to low priority
        ET1 = 1; // enables Timer 1 interrupt
        TR1 = 1; // starts Timer 1
    }
    
    autopoll_enabled = en_autopoll;

    // init states
    preform_poll = false;

    // inputs are active high on this system.
    current_button_state.raw = UINT32_MAX;

    for (int i = 0; i < PIUIO_NUM_SENSORS; i++)
    {
        all_states[PIUIO_NUM_SENSORS].raw = UINT32_MAX;
    }

    current_sensor_num = 0;
    set_sensor_mux();
}

void push_lights(piuio_output_state_t *light_state)
{
    // disable interrupts.
    EA = 0;

    // set to output
    OEA = 0xFF;
    OEB = 0xFF;

    // push first half of state.
    IOA = light_state->buff[0];
    IOB = light_state->buff[1];

    // keep clock high for ~1us
    // latches on rising.
    PD4 = 1;
    delay_4c(DELAY_OUTPUT_4C);
    PD4 = 0;
    delay_4c(DELAY_OUTPUT_4C);

    // push last half of state.
    IOA = light_state->buff[2];
    IOB = light_state->buff[3];

    // repeat
    PD5 = 1;
    delay_4c(DELAY_OUTPUT_4C);
    PD5 = 0;
    delay_4c(DELAY_OUTPUT_4C);

    // original decomp does this, so repeating.
    IOA = 0xFF;
    IOB = 0xFF;

    IOA = 0x00;
    IOB = 0x00;

    // return to input.
    OEA = 0x00;
    OEB = 0x00;

    // reenable interrupts.
    EA = 1;
}

void mux_lamp_state(piuio_output_state_t *light_state_from_game)
{
    // take note of this for later to mux it with the reactive lighting.
    current_game_lamp_state.raw = light_state_from_game->raw;

    current_lamp_state.raw = light_state_from_game->raw;

    // inject our sensor mux positions.
    set_sensor_mux();
}

piuio_input_state_t get_input_state(void)
{
    piuio_input_state_t new_state;
    new_state.raw = 0;

    // disable interrupts.
    EA = 0;

    // turn into input.
    OEA = 0x00;
    OEB = 0x00;
    delay_4c(DELAY_INPUT_4C);

    // pulling low latches the input.
    PD1 = 0;
    delay_4c(DELAY_INPUT_4C);

    new_state.buff[3] = IOB;
    new_state.buff[2] = IOA;

    PD1 = 1;
    delay_4c(DELAY_INPUT_4C);

    // repeat
    PD0 = 0;
    delay_4c(DELAY_INPUT_4C);

    new_state.buff[1] = IOB;
    new_state.buff[0] = IOA;

    PD0 = 1;
    delay_4c(DELAY_INPUT_4C);

    // original firmware also toggles PD2, but it's not used on the final board
    // so skip.

    // reenable interrupts.
    EA = 1;

    return new_state;
}

void io_task(void)
{
    if (autopoll_enabled)
    {
        if (preform_poll)
        {
            create_exchange();

            preform_poll = false;
        }
    }
}