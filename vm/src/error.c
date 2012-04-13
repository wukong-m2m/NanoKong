//
//  NanoVM, a tiny java VM for the Atmel AVR family
//  Copyright (C) 2005 by Till Harbaum <Till@Harbaum.org>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// 

#if defined(UNIX) || defined(__CC65__)
#include <stdio.h>
#include <stdlib.h>
#else
#include <avr/io.h>
#include "delay.h"
#endif

#ifdef NIBOBEE
#include <util/delay.h>
#endif

#include "types.h"
#include "config.h"
#include "debug.h"
#include "error.h"

#include "nvmcomm.h"

#if defined(UNIX) || defined(__CC65__)
char *error_msg[] = {
  // unix message              avr error code
  "HEAP: illegal chunk size",        // A
  "HEAP: corrupted",                 // B
  "HEAP: out of memory",             // C
  "HEAP: chunk does not exist",      // D
  "HEAP: out of stack memory",       // E
  "HEAP: stack underrun",            // F
  "HEAP: out of identifiers",        // G
  "ARRAY: illegal type",             // H
  "NATIVE: unknown method",          // I
  "NATIVE: unknown class",           // J
  "NATIVE: illegal argument",        // K
  "NVMFILE: unsupported features or not a valid nvm file",   // L
  "NVMFILE: wrong nvm file version", // M
  "VM: illegal reference",           // N
  "VM: unsupported opcode",          // O
  "VM: division by zero",            // P
  "VM: stack corrupted",             // Q
  "WKPF: initialisation failed",     // R
};
#else
#include "uart.h"
#endif

void error(err_t code) {
#if defined(UNIX) || defined(__CC65__)
  printf("NanoVM error: %s\n", error_msg[code]);
  exit(-1);
#else

  uart_putc(0, 'E');
  uart_putc(0, 'R');
  uart_putc(0, 'R');
  uart_putc(0, ':');
  uart_putc(0, 'A'+code);
  uart_putc(0, '\n');

  for(;;) {
    // reset watchdog here if in use
    
    // Check if there's any packet coming in that we need to handle before processing the next VM instruction.
    // Need to do this here because the VM's main loop is stopped.
    // TODO: Reconsider this when we have a better design for receiving messages.
    nvmcomm_poll();
    delay(MILLISEC(10));

#ifdef ASURO
    // yellow/red blinking status led
    PORTD |= _BV(2);
    PORTB |= _BV(0);
    delay(MILLISEC(250));
    PORTB &= ~_BV(0);
    delay(MILLISEC(250));
#endif

#ifdef NIBOBEE
    uint8_t cnt;
    for (cnt=5; cnt; cnt--) {
      PORTB &= 0xf0;
      PORTB |= _BV(1);
      _delay_ms(100);
      PORTB &= 0xf0;
      PORTB &= _BV(2);
      _delay_ms(100);
    }
    PORTB &= 0xf0;
    _delay_ms(1000);
    PORTB |= 0x0f&code;
    _delay_ms(1000);
#endif
  }

  
#endif
}
