#ifndef __PINS_DOT_H__
#define __PINS_DOT_H__

/* note that the code in main.c and necir.c will need the PORTn and DDRn macros
   updating if you move a pin to a different port. You can more easily move the
   pins to another bit in the same port just by updating these defines. 
*/

// inputs
#define PIN_AMP_ON   PD7
#define PIN_IR_RX    PD2
#define PIN_DAC_ON   PD3

// outputs
#define PIN_RELAY    PB1
#define PIN_USER_LED PB5
#define PIN_IR_TX    PD4

#endif
