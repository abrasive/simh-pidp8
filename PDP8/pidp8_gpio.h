/*
 * pidp8_gpio.h: moving parts for blinking lights
 */

#include <stdint.h>

typedef unsigned int ui;

typedef union {
    uint16_t raw[3];
    struct {
        // row 1
        ui SR        : 12;
        ui _pad0     : 4;  // to 16 bits
        // row 2
        ui _unused1  : 6;
        ui IF        : 3;
        ui DF        : 3;
        ui _pad1     : 4;
        // row 3
        ui _unused2  : 4;
        ui SING_INST : 1;
        ui SING_STEP : 1;
        ui STOP      : 1;
        ui CONT      : 1;
        ui EXAM      : 1;
        ui DEP       : 1;
        ui LOAD_ADD  : 1;
        ui START     : 1;
        ui _pad2     : 4;
    };
} pidp_switch_t;

typedef union {
    uint16_t raw[8];
    struct {
        uint16_t PC;    // row 1
        uint16_t MA;    // row 2
        uint16_t MB;    // row 3
        uint16_t AC;    // row 4
        uint16_t MQ;    // row 5
        // row 6
        ui WRDCT    : 1;   // aka WC
        ui DEFER    : 1;
        ui EXEC     : 1;
        ui FETCH    : 1;
        ui OPR      : 1;
        ui IOT      : 1;
        ui JMP      : 1;
        ui JMS      : 1;
        ui DCA      : 1;
        ui ISZ      : 1;
        ui TAD      : 1;
        ui AND      : 1;
        ui _pad6    : 4;
        // row 7
        ui _unused7 : 2;
        ui SC       : 5;
        ui RUN      : 1;
        ui PAUSE    : 1;
        ui ION      : 1;
        ui BREAK    : 1;
        ui CURAD    : 1;   // aka CA
        ui _pad7    : 4;
        // row 8
        ui _unused8 : 5;
        ui LINK     : 1;
        ui IF       : 3;
        ui DF       : 3;
        ui _pad8    : 4;
    };
} pidp_led_t;

extern pidp_switch_t switches, switches_event;
extern pidp_led_t leds;

void *blink(int *terminate);      // LED/switch driver thread function
