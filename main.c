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
#include "version.h"

/* pins
 * D9  (PB1) - relay coil (via NPN transistor)
 * D7  (PD7) - input from power amp (via opto-isolator)
 * D2  (PD2) - IR receiver input
 * D13 (PB5) - LED on nano board
 */

void relay_on(void)
{
    report("Relay: ON\n");
    PORTB |= _BV(PB1);
}

void relay_off(void)
{
    report("Relay: OFF\n");
    PORTB &= ~_BV(PB1);
}

void led_on(void)
{
    PORTB |= _BV(PB5);
}

void led_off(void)
{
    PORTB &= ~_BV(PB5);
}

bool is_amp_powered_on(void)
{
    return (PIND & _BV(PD7));
}

void amp_on(void)
{
    if(is_amp_powered_on()){
        report("Amplifier: already ON\n");
        return;
    }
    report("Amplifier: ON\n");
    relay_on();
    _delay_ms(100);
    relay_off();
}

void amp_off(void)
{
    if(!is_amp_powered_on()){
        report("Amplifier: already OFF\n");
        return;
    }
    report("Amplifier: OFF\n");
    relay_on();
    _delay_ms(100);
    relay_off();
}

int main(void)
{
    bool power_amp_on, last_power_amp_on = false;
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
    PORTB &= ~(_BV(PB1));                            // disable pull-up
    DDRB  |=  (_BV(PB1));                            // set as output

    // setup LED output pin
    PORTB &= ~(_BV(PB5));                            // disable pull-up
    DDRB  |=  (_BV(PB5));                            // set as output

    // setup amp power input pin
    PORTD |=  (_BV(PD7));                            // enable pull-up
    DDRD  &= ~(_BV(PD7));                            // set as input

    // initialise RC5 library
    RC5_Init();

    last_power_amp_on = !is_amp_powered_on();

    while(1){
        wdt_reset();
        debug_periodic();

        power_amp_on = is_amp_powered_on();
        if(power_amp_on != last_power_amp_on){
            report("Power amp is %s\n", power_amp_on?"ON":"OFF");
            last_power_amp_on = power_amp_on;
        }

        serial_in = serial_read_byte();
        switch(serial_in){
            case 'n':
            case 'N':
                amp_on();
                break;
            case 'f':
            case 'F':
                amp_off();
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
                led_off_timer = 5000;
                led_on();

                report("RC5 command: address=%d, command=%d, toggle=%d\n",
                        RC5_GetAddressBits(rc5_command),
                        RC5_GetCommandBits(rc5_command),
                        RC5_GetToggleBit(rc5_command));
                if(RC5_GetAddressBits(rc5_command) == 20){
                    if(RC5_GetCommandBits(rc5_command) == 5){
                        amp_on();
                    }
                    if(RC5_GetCommandBits(rc5_command) == 6){
                        amp_off();
                    }
                }
            }
        }
    }

    return 0;
}
