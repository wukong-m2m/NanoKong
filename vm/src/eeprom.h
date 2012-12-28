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

#ifndef EEPROM_H
#define EEPROM_H

//typedef u08_t* eeprom_addr_t;

#ifdef UNIX
#include <string.h> // for memcpy

#define eeprom_read_byte(a) (*a)
#define eeprom_update_byte(a, d) { *a = d; }
#define eeprom_read_block(a, s, l) { memcpy(a, s, l); }
#define eeprom_update_block(a, s, l) { memcpy(s, a, l); }
#define EEPROM

#else // UNIX
#ifdef AVR

// TH: Otherwise gcc-3.4.5/avr-lib-1.4.2 complain about unknow
// symbol asm ... is there a more beautiful solution?
#define asm __asm__

#include <avr/eeprom.h>
#define EEPROM EEMEM

// Some older versions of the AVR libraries don't have the eeprom_update_ functions.
// In that case just use the eeprom_write_ version instead
#ifndef eeprom_update_byte
#define eeprom_update_byte eeprom_write_byte
#endif

#ifndef eeprom_update_block
#define eeprom_update_block eeprom_write_block
#endif


#else // AVR

#error "Unknown EEPROM setup"

#endif // AVR
#endif // UNIX

#else // NVM_USE_EEPROM

#define EEPROM

#endif // EEPROM_H
