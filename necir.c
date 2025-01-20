#include "necir.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>
#include "pins.h"

/* ask delay.h to prefer shorter delays, since we have some CPU overhead also */
#define __DELAY_ROUND_DOWN__ 1
#include <util/delay.h>

/*
   This is a very simple NEC IR protocol tranmission routine.
   CPU generates PWM with delay loops.
*/

static inline void nec_led_on(void)
{
    /* IR transmitter LED on */
    PORTD |= _BV(PD4);
}

static inline void nec_led_off(void)
{
    /* IR transmitter LED off */
    PORTD &= ~_BV(PD4);
}

/* this time is half a cycle of the 38KHz carrier */
#define BASIC_DELAY (1000000.0/38000.0/2.0) /* note that delay_us expects a floating point argument */

static void nec_burst_1(void)
{
    /* send about 562.5us of 38kHz */
    for(uint8_t n=0; n<21; n++){
        nec_led_on();
        _delay_us(BASIC_DELAY);
        nec_led_off();
        _delay_us(BASIC_DELAY);
    }
}

static void nec_burst_0(void)
{
    /* send about 562.5us of silence */
    for(uint8_t n=0; n<21; n++){
        nec_led_off(); /* retain in case this affects timing */
        _delay_us(BASIC_DELAY);
        nec_led_off();
        _delay_us(BASIC_DELAY);
    }
}

static void nec_encode_byte(bool *bitarray, uint8_t data)
{
    /* Bytes are sent MSB first */
    for(uint8_t n=0; n<8; n++){
        *bitarray = (data & 0x80) ? true : false;
        bitarray++;
        data = data << 1;
    }
}

void send_nec_ir(uint8_t address, uint8_t command)
{
    bool message[32];

    /*
       Encode the message in advance so the transmit loop is very simple.
       The standard NEC message format is 32 bits long:
    */
    nec_encode_byte(&message[0], address);   /* address byte */
    nec_encode_byte(&message[8], ~address);  /* inverted address byte */
    nec_encode_byte(&message[16], command);  /* command byte */
    nec_encode_byte(&message[24], ~command); /* inverted command byte */

    /* Send 9ms of 38kHz */
    for(uint8_t n=0; n<16; n++){
        nec_burst_1();
    }

    /* Then 4.5ms of silence */
    for(uint8_t n=0; n<8; n++){
        nec_burst_0();
    }

    /* Then the data stream */
    for(uint8_t n=0; n<32; n++){
        /* 0 and 1 bits start the same */
        nec_burst_1();
        nec_burst_0();
        /* 1 bits are twice the duration of 0 bits */
        if(message[n]){
            nec_burst_0();
            nec_burst_0();
        }
    }

    /* Then a final end bit */
    nec_burst_1();

    /* Make sure we don't burn out the LED */
    nec_led_off();
}

void send_nec_repeat(void) // *UNTESTED*
{
    /*
       NEC protocol for repeat
       This should be sent every 110ms after the initial code began, while the key is held down
    */

    /* Send 9ms of 38kHz */
    for(uint8_t n=0; n<16; n++){
        nec_burst_1();
    }

    /* Then 2.25ms silence */
    for(uint8_t n=0; n<4; n++){
        nec_burst_0();
    }

    /* Then a final end bit */
    nec_burst_1();

    /* Make sure we don't burn out the LED */
    nec_led_off();
}

/* vim:set shiftwidth=4 expandtab: */
