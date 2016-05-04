/*
 * pidp8_gpio.h: moving parts for blinking lights
 */

#include <stdint.h>

extern uint32_t switchstatus[3];  // bitfields: 3 rows of up to 12 switches
extern uint32_t ledstatus[8];     // bitfields: 8 ledrows of up to 12 LEDs

void *blink(int *terminate);      // LED/switch driver thread function
