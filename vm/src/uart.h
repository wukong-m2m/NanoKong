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

#ifndef UART_H
#define UART_H

extern void uart_init(u08_t uart, u32_t baudrate);

//#if defined(ATMEGA2560)
extern void uart_setBaudrate(u08_t uart, u32_t baudrate, u08_t factor);
extern void uart_setStopbit(u08_t uart, u08_t stopbit);
extern void uart_setParity(u08_t uart, u08_t parity);
extern void uart_flush(u08_t uart);
extern void uart_spi_begin(u08_t uart, u32_t baudrate);
extern u08_t uart_spi_transfer(u08_t uart, u08_t byte);
//#endif

extern void uart_write_byte(u08_t uart, u08_t byte);
extern void uart_putc(u08_t uart, u08_t byte);
extern u08_t uart_read_byte(u08_t uart);
extern u08_t uart_available(u08_t uart);

#endif // UART_H
