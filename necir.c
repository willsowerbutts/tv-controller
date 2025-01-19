/* vim:set shiftwidth=4 expandtab: */

#include "necir.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>

/* ask delay.h to prefer shorter delays, since we're already delaying too little due to CPU overhead */
#define __DELAY_ROUND_DOWN__ 1
#include <util/delay.h>

// This is a very simple NEC IR protocol tranmission routine. 
// CPU generates PWM with delay loops.

static inline void nec_led_on(void)
{
    PORTD |= _BV(PD4);
}

static inline void nec_led_off(void)
{
    PORTD &= ~_BV(PD4);
}

/* this time is half a cycle of the 38KHz carrier */
#define BASIC_DELAY (1000000.0/38000.0/2.0) /* note that delay_us expects a floating point argument */

// send about 562.5us of 38kHz
void nec_burst_1(void)
{
    for(uint8_t n=0; n<21; n++){
        nec_led_on();
        _delay_us(BASIC_DELAY);
        nec_led_off();
        _delay_us(BASIC_DELAY);
    }
}

// send about 562.5us of silence
void nec_burst_0(void)
{
    for(uint8_t n=0; n<21; n++){
        nec_led_off();
        _delay_us(BASIC_DELAY);
        nec_led_off();
        _delay_us(BASIC_DELAY);
    }
}

void send_nec_ir(uint8_t address, uint8_t command)
{
    uint32_t message;
    bool bit;

    message = address;                                  // first byte
    message = (message << 8) | (address ^ 0xFF);        // second byte
    message = (message << 8) | command;                 // third byte
    message = (message << 8) | (command ^ 0xFF);        // fourth byte

    // NEC protocol:

    // Send 9ms of 38kHz
    for(uint8_t n=0; n<16; n++){
        nec_burst_1();
    }

    // Then 4.5ms of silence
    for(uint8_t n=0; n<8; n++){
        nec_burst_0();
    }

    // Then the data stream, MSB first
    for(uint8_t n=0; n<32; n++){
        bit = message & 0x80000000;
        message <<= 1;
        // 0 and 1 bits start the same
        nec_burst_1();
        nec_burst_0();
        if(bit){ // 1 bits are twice the duration
            nec_burst_0();
            nec_burst_0();
        }
    }

    // Then a final end bit
    nec_burst_1();

    // Make sure we don't burn out the LED
    nec_led_off();
}

void send_nec_repeat(void) // *UNTESTED*
{
    // NEC protocol for repeat
    // This should be sent every 100ms after the initial code, while the key is held down

    // Send 9ms of 38kHz
    for(uint8_t n=0; n<16; n++){
        nec_burst_1();
    }

    // Then 2.25ms silence 
    nec_burst_0();
    nec_burst_0();
    nec_burst_0();
    nec_burst_0();

    // Then a final end bit
    nec_burst_1();

    // Make sure we don't burn out the LED
    nec_led_off();
}
