#include <stdio.h>
#include "avr/io.h"
#include "delay.h"
#include "debug.h"
#include "logging.h"
#include "led.h"

#define INTERVAL 0.25

// Some macros that make the code more readable
#define output_low(port,pin) port &= ~(1<<pin)
#define output_high(port,pin) port |= (1<<pin)
#define set_input(portdir,pin) portdir &= ~(1<<pin)
#define set_output(portdir,pin) portdir |= (1<<pin)

void blink_once(LED which)
{
    blink(which, 1, INTERVAL);
}

void blink_twice(LED which)
{
    blink(which, 2, INTERVAL);
}

void blink_thrice(LED which)
{
    blink(which, 3, INTERVAL);
}

void blink(LED which, uint8_t times, uint8_t interval)
{
    set_output(DDRK, which);
#ifdef LOGGING
    char message[25];
    uint8_t n = sprintf(message, "blinking LED%x %d times", which+4, times);
    LOGF(message, n);
#endif
#ifdef DEBUG
    DEBUGF_GROUP("blinking LED%d %x times\n", which+4, times);
#endif
    for (int i=0; i<times; ++i) {
        output_low(PORTK, (int)which);
        delay(MILLISEC(interval));
        output_high(PORTK, (int)which);
    }
}
