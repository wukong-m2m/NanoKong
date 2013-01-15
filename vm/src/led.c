#include "avr/io.h"
#include "delay.h"
#include "led.h"

#define INTERVAL 250

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
    for (int i=0; i<times; ++i) {
        output_low(PORTK, which);
        delay(MILLISEC(interval));
        output_high(PORTK, which);
    }
}
