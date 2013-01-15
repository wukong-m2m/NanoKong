#include "avr/io.h"
#include "led.h"

#define INTERVAL 250

// Some macros that make the code more readable
#define output_low(port,pin) port &= ~(1<<pin)
#define output_high(port,pin) port |= (1<<pin)
#define set_input(portdir,pin) portdir &= ~(1<<pin)
#define set_output(portdir,pin) portdir |= (1<<pin)

void delay_ms(uint16_t ms) {
    uint16_t delay_count = F_CPU / 17500;
    volatile uint16_t i;

    while (ms != 0) {
        for (i=0; i!=delay_count; i++);
        ms--;
    }
}

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
        delay_ms(interval);
        output_high(PORTK, which);
    }
}
