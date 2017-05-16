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
#include "version.h"

int main(void)
{
    // unsigned char inputs, prev_inputs;
    // unsigned long outputA_timer, outputB_timer;

    // enable the watchdog timer
    wdt_enable(WDTO_4S);

    // initialise serial
    serial_init();
    debug_init();

    // enable interrupts (they are all masked initially)
    sei();

    // // setup input pins
    // PORTD |=  (_BV(PD2) | _BV(PD3) | _BV(PD4) | _BV(PD5));      // enable built-in pull-ups
    // DDRD  &= ~(_BV(PD2) | _BV(PD3) | _BV(PD4) | _BV(PD5));      // set as inputs

    // // setup output pins
    // PORTB &= ~(_BV(PB1) | _BV(PB2));                            // disable pull-ups
    // DDRB  |=  (_BV(PB1) | _BV(PB2));                            // set as outputs

    // announce ourselves
    report("\nPower Amplifier IR control module version %S.\n\n", software_version_string);

    // inputs = prev_inputs = 0;
    // outputA_timer = 0;
    // outputB_timer = 0;

    // while(1){
    //     wdt_reset();
    //     debug_periodic();

    //     // read inputs
    //     inputs = PIND & (_BV(PD2) | _BV(PD3) | _BV(PD4) | _BV(PD5));

    //     if((inputs & _BV(PD2)) != (prev_inputs & _BV(PD2))){
    //         // PD2 changed
    //         if(inputs & _BV(PD2)){
    //             outputA_timer = TIMER_RESET_VALUE;
    //         }else{
    //             outputA_timer = 0;
    //         }
    //     }
    //     if( ((inputs & _BV(PD3)) != (prev_inputs & _BV(PD3))) ||
    //         ((inputs & _BV(PD4)) != (prev_inputs & _BV(PD4))) ||
    //         ((inputs & _BV(PD5)) != (prev_inputs & _BV(PD5)))){
    //         // PD3,4,5 changed
    //         if(inputs & (_BV(PD3) | _BV(PD4) | _BV(PD5))){
    //             outputB_timer = TIMER_RESET_VALUE;
    //         }else{
    //             outputB_timer = 0;
    //         }
    //     }

    //     prev_inputs = inputs;

    //     if(outputA_timer){
    //         PORTB |= _BV(PB1);
    //         outputA_timer--;
    //     }else
    //         PORTB &= ~_BV(PB1);

    //     if(outputB_timer){
    //         PORTB |= _BV(PB2);
    //         outputB_timer--;
    //     }else
    //         PORTB &= ~_BV(PB2);

    //     _delay_ms(1);
    // }

    return 0;
}
