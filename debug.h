#ifndef __DEBUG_DOT_H__
#define __DEBUG_DOT_H__

#include <stdio.h>
#include <avr/pgmspace.h>

#ifdef DEBUG
#define report(msg, args...) do { printf_P( PSTR(msg), ## args); } while(0)
void debug_dumpmem(void *ptr, uint16_t len);
#define dumpmem(ptr, len) do { debug_dumpmem(ptr, len); } while(0)
// void debug_serial(char *fmt, ...); // primarily required in the networking stack where report() can recurse
#else
#define report(msg, args...) do { } while(0)
#define dumpmem(ptr, len) do { } while(0)
#define debug_serial(msg, args...) do { } while(0)
#endif

void debug_init(void);
void debug_periodic(void);
void debug_flush_buffer(void);

#endif
