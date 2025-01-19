/* vim:set shiftwidth=4 expandtab: */

#include "necir.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>
#include "pins.h"

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
static void nec_burst_1(void)
{
    for(uint8_t n=0; n<21; n++){
        nec_led_on();
        _delay_us(BASIC_DELAY);
        nec_led_off();
        _delay_us(BASIC_DELAY);
    }
}

// send about 562.5us of silence
static void nec_burst_0(void)
{
    for(uint8_t n=0; n<21; n++){
        nec_led_off();
        _delay_us(BASIC_DELAY);
        nec_led_off();
        _delay_us(BASIC_DELAY);
    }
}

static void nec_encode_byte(bool *bitarray, uint8_t data)
{
    for(uint8_t n=0; n<8; n++){
        *bitarray = (data & 0x80) ? true : false;
        bitarray++;
        data <<= 1;
    }
}

void send_nec_ir(uint8_t address, uint8_t command)
{
    /* 
       Encode the message in advance so the transmit loop is very simple.
       The standard NEC message format is 32 bits long:
        - address byte
        - inverse of address byte
        - command byte
        - inverse of command byte
    */
    bool message[32];

    nec_encode_byte(&message[0], address);
    nec_encode_byte(&message[8], ~address);
    nec_encode_byte(&message[16], command);
    nec_encode_byte(&message[24], ~command);

    // NEC protocol:

    // Send 9ms of 38kHz carrier
    for(uint8_t n=0; n<16; n++){
        nec_burst_1();
    }

    // Then 4.5ms of silence
    for(uint8_t n=0; n<8; n++){
        nec_burst_0();
    }

    // Then the data stream, MSB first
    for(uint8_t n=0; n<32; n++){
        // 0 and 1 bits start the same
        nec_burst_1();
        nec_burst_0();
        if(message[n]){ // 1 bits are twice the duration of 0 bits
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
