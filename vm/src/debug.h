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

//  
//  debug.h
//

#ifndef DEBUG_H
#define DEBUG_H

#include "config.h"
#include "types.h"

#define DBG8 "%02x"
#define DBG16 "%04x"
#define DBG32 "%08lx"

#define DEBUG_UART 0

#ifdef DEBUG
// Turn off specific DEBUGF types by commenting "debugf(__VA_ARGS__)"
#define DEBUGF_INSTR(...) // debugf(__VA_ARGS__)
#define DEBUGF_HEAP(...) // debugf(__VA_ARGS__) // Heap operations
#define DEBUGF_COMM(...) // debugf(__VA_ARGS__) // Nvmcomm
#define DEBUGF_ZWAVETRACE(...) // debugf(__VA_ARGS__) // Incoming zwave traffic per byte
#define DEBUGF_FLASH(...) // debugf(__VA_ARGS__) // avr_flash.c
#define DEBUGF_USART(...) // debugf(__VA_ARGS__) 
#define DEBUGF_XBEE(...) // debugf(__VA_ARGS__) 
#define DEBUGF_READFLASH(...) // debugf(__VA_ARGS__) 
#define DEBUGF_WKPF(...)  debugf(__VA_ARGS__) 
#define DEBUGF_TEST(...)  debugf(__VA_ARGS__) 
#define DEBUGF(...) // debugf(__VA_ARGS__) // All other debug info.
#define DEBUG_HEXDUMP(a,b) debug_hexdump(a,b)
void debugf(const char *fmt, ...);
void debug_hexdump(const void *data, u16_t size);
#else
#define DEBUGF_INSTR(...)
#define DEBUGF_HEAP(...)
#define DEBUGF_COMM(...)
#define DEBUGF_ZWAVETRACE(...)
#define DEBUGF_FLASH(...)
#define DEBUGF_USART(...)
#define DEBUGF_XBEE(...)
#define DEBUGF_READFLASH(...)
#define DEBUGF_WKPF(...)
#define DEBUGF_TEST(...)
#define DEBUGF(...)
#define DEBUG_HEXDUMP(a,b)
#endif

void debug_enable(bool_t enable);

#endif //DEBUG_H
