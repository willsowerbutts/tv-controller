#include <stdbool.h>
#include <avr/io.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "debug.h"
#include "serial.h"
#include "rc5.h"
#include "necir.h"
#include "version.h"
#include "pins.h"

/* pins
 * D9  (PB1) - relay coil (via NPN transistor)
 * D7  (PD7) - LED input from power amp (via opto-isolator)
 * D2  (PD2) - IR receiver input (via opto-isolator)
 * D3  (PD3) - 12V trigger from DAC (via opto-isolator)
 * D4  (PD4) - IR output (to IR blaster)
 * D13 (PB5) - LED on nano board
 *
 * PCB connectors:
 *        +-------------------------------------------+
 *        | > AMP LED +                    IR OUT + < |
 *        | > AMP LED -                    IR OUT - < |
 *        | > AMP SW                        IR IN + < |
 *        | > AMP SW                        IR IN - < |
 *        |                                           |
 *   +----------------+             [OPTOCOUPLER IC]  |
 *   |                |                               |
 *   |    5V RELAY    |               12V TRIGGER - < |
 *   |                |               12V TRIGGER + < |
 *   +----------------+-------------------------------+
 */

uint16_t amp_off_timer;

static void relay_on(void)
{
    report("Relay: ON\n");
    PORTB |= _BV(PIN_RELAY);
}

static void relay_off(void)
{
    report("Relay: OFF\n");
    PORTB &= ~_BV(PIN_RELAY);
}

static void led_on(void)
{
    PORTB |= _BV(PIN_USER_LED);
}

static void led_off(void)
{
    PORTB &= ~_BV(PIN_USER_LED);
}

static bool is_amp_powered_on(void)
{
    return (PIND & _BV(PIN_AMP_ON));
}

static bool is_dac_powered_on(void)
{
    return ! (PIND & _BV(PIN_DAC_ON));
}

static void amp_on(void)
{
    if(is_amp_powered_on()){
        report("Amplifier: already ON\n");
        return;
    }
    report("Amplifier: ON\n");
    relay_on();
    _delay_ms(100);
    relay_off();
    amp_off_timer = 0;
}

static void amp_off(void)
{
    if(!is_amp_powered_on()){
        report("Amplifier: already OFF\n");
        return;
    }
    report("Amplifier: OFF\n");
    relay_on();
    _delay_ms(100);
    relay_off();
    amp_off_timer = 0;
}

static void amp_off_delay(void)
{
    if(!is_amp_powered_on()){
        report("Amplifier: already OFF\n");
        return;
    }
    report("Amplifier: delayed off ...\n");
    amp_off_timer = 30000; // a bit more than 0.1 millisecond each
}

static void amp_off_delay_check(void)
{
    switch(amp_off_timer){
        case 0: 
            return;             // no timer running
        case 1: 
            amp_off();          // on the final cycle, turn it off 
                                // fall through
        default:
            _delay_us(100);     // delay a little each loop
            amp_off_timer--;    // decrement timer
    }
}

/* Topping E70 DAC:
 * Control codes can be found here
 * https://www.audiosciencereview.com/forum/index.php?threads/remote-codes-for-topping.10708/
 * Topping RC-15A
 * Power:  0x11 0x18
 * Mute:   0x11 0x60
 * Vol +:  0x11 0x62
 * Vol -:  0x11 0x68
 * Left:   0x11 0xE2
 * Right:  0x11 0xA8
 * A:      0x11 0x20
 * B:      0x11 0x02
 * C1:     0x11 0x2A
 * C2:     0x11 0x0A
 * Gain:   0x11 0x08
 * Dim:    0x11 0x28
 */

static void vol_up(void)
{
    send_nec_ir(0x11, 0x62);
    report("+");
}

static void vol_down(void)
{
    send_nec_ir(0x11, 0x68);
    report("-");
}

static void vol_mute(void)
{
    send_nec_ir(0x11, 0x60);
    report("[mute]");
}

int main(void)
{
    bool amp_power_on, last_amp_power_on = false;
    bool dac_power_on, last_dac_power_on = false;
    uint16_t rc5_command;
    uint16_t led_off_timer = 0;
    int serial_in;

    // enable the watchdog timer
    wdt_enable(WDTO_4S);

    // initialise serial
    serial_init();
    debug_init();

    // enable interrupts (they are all masked initially)
    sei();

    // announce ourselves
    report("\nPower Amplifier IR control module version %S.\n\n", software_version_string);

    // ensure startup is in the desired state
    relay_off();
    led_off();

    // setup relay output pin
    PORTB &= ~(_BV(PIN_RELAY));                      // disable pull-up
    DDRB  |=  (_BV(PIN_RELAY));                      // set as output

    // setup user LED output pin
    PORTB &= ~(_BV(PIN_USER_LED));                   // disable pull-up
    DDRB  |=  (_BV(PIN_USER_LED));                   // set as output

    // setup IR transmitter LED output pin
    PORTD &= ~(_BV(PIN_IR_TX));                      // disable pull-up
    DDRD  |=  (_BV(PIN_IR_TX));                      // set as output

    // setup IR input pin
    PORTD |=  (_BV(PIN_IR_RX));                      // enable pull-up
    DDRD  &= ~(_BV(PIN_IR_RX));                      // set as input

    // setup amp power input pin
    PORTD |=  (_BV(PIN_AMP_ON));                     // enable pull-up
    DDRD  &= ~(_BV(PIN_AMP_ON));                     // set as input

    // setup DAC power input pin
    PORTD |=  (_BV(PIN_DAC_ON));                     // enable pull-up
    DDRD  &= ~(_BV(PIN_DAC_ON));                     // set as input

    // initialise RC5 library -- this sets up the PIN_IR_RX for us
    RC5_Init();

    // set these wrong so it forces a report at startup
    last_amp_power_on = !is_amp_powered_on();
    last_dac_power_on = !is_dac_powered_on();

    while(1){
        wdt_reset();
        debug_periodic();

        amp_power_on = is_amp_powered_on();
        if(amp_power_on != last_amp_power_on){
            report("Power amp is %s\n", amp_power_on?"ON":"OFF");
            last_amp_power_on = amp_power_on;
        }

        dac_power_on = is_dac_powered_on();
        if(dac_power_on != last_dac_power_on){
            report("DAC is %s\n", dac_power_on?"ON":"OFF");
            last_dac_power_on = dac_power_on;
            // when the DAC changes power state, do the same for the power amp
            if(dac_power_on)
                amp_on();
            else
                amp_off_delay();
        }

        amp_off_delay_check();

        serial_in = serial_read_byte();
        switch(serial_in){
            case 'n':
            case 'N':
            case '1':
                amp_on();
                break;
            case 'f':
            case 'F':
            case '0':
                amp_off();
                break;
            case 'd':
            case 'D':
                amp_off_delay();
                break;
            case 'm':
            case 'M':
                vol_mute();
                break;
            case '-':
                vol_down();
                break;
            case '+':
                vol_up();
                break;
            default:
                break;
        }

        if(led_off_timer){
            if(--led_off_timer == 0)
                led_off();
        }

        if(RC5_NewCommandReceived(&rc5_command)){
            RC5_Reset(); // prepare for next command
            if(RC5_GetStartBits(rc5_command) != 3){
                report("RC5 command: BAD -- %d start bits\n", RC5_GetStartBits(rc5_command));
            }else{
                bool report_msg = true;

                if(RC5_GetAddressBits(rc5_command) == 16){
                    led_on();
                    led_off_timer = 5000;
                    switch(RC5_GetCommandBits(rc5_command)){
                        case 17: vol_down(); report_msg = false; break;
                        case 16: vol_up();   report_msg = false; break;
                        case 13: vol_mute(); report_msg = false; break;
                    }
                }

                if(report_msg){
                    report("RC5 addr %d, cmd %d, tog %d\n",
                            RC5_GetAddressBits(rc5_command),
                            RC5_GetCommandBits(rc5_command),
                            RC5_GetToggleBit(rc5_command));
                }
            }
        }
    }

    return 0;
}

/* vim:set shiftwidth=4 expandtab: */
