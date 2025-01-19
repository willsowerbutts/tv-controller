#ifndef NECIR_H
#define NECIR_H

#include <stdint.h>

void send_nec_ir(uint8_t address, uint8_t command);
void send_nec_repeat(void);

#define NEC_CARRIER_FREQUENCY 38000

#endif
