#ifndef LED_H
#define LED_H

typedef enum {
    LED4,
    LED5,
    LED6,
    LED7
} LED;

void blink_once(LED which);
void blink_twice(LED which);
void blink_thrice(LED which);
void blink(LED which, uint8_t times, uint8_t interval);

#endif
