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
//  main() for NanoVM runtime
//

#include <stdio.h>

#include "types.h"
#include "config.h"
#include "debug.h"

#include "uart.h"
#include "nvmfile.h"
#include "vm.h"
#include "nvmcomm.h"

#include "avr/avr_flash.h"
#include <avr/boot.h>

#ifdef ATMEGA168
#include <avr/wdt.h>
#endif

#ifdef TEST_WKPF
#include "tests/test_wkpf.h"
#endif

// hooks for init routines

#include "native_impl.h"

#ifdef LCD
#include "native_lcd.h"
#endif

int main(int argc, char **argv) {
#ifdef ATMEGA168
  // For some reason, after a WDT reset, WDT persists for some CPU's
  // like ATmega168 and wdt_disable alone is not sufficient to disable
  // the WDT again (avr-libc bug?). Therefore:
  MCUSR &= ~(1<<WDRF);
  wdt_disable();
#endif

#if defined(UNIX) || defined(NVM_USE_COMM)
  uart_init(0, UART_BAUDRATE);
#endif

#ifdef DEBUG // TODO: temporary
  debug_enable(TRUE);
#endif

#if defined(NVM_USE_COMM)
  nvmcomm_init();
#endif

// call native initialization (e.g. hardware setup)
#ifdef NATIVE_INIT
  NATIVE_INIT;
#endif

  nvmfile_init();

  vm_init();

#ifdef TEST_WKPF
  test_wkpf();
#endif

  DEBUGF("NanoVM\n");

  nvmfile_call_main();

#ifdef UNIX
  // the following is only being done to detect memory leaks and
  // heap corruption and can thus be omitted on the real thing (tm)

  // give main args back to heap
  heap_unsteal(1*sizeof(nvm_stack_t));

  heap_garbage_collect();
  heap_show();
#endif // UNIX

  DEBUGF("main() returned\n");

#if !defined(UNIX) && !defined(__CC65__)
  // don't care for anything after this ...
  for(;;);  // reset wdt if in use
#endif

  return 0;
}
