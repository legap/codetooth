/*
  Copyright 2010-07 By Opendous Inc. (www.MicropendousX.org)
  NVIC handler info copied from NXP User Manual UM10360

  Start-up code for LPC17xx.  See TODOs for
  modification instructions.

  Permission to use, copy, modify, and distribute this software
  and its documentation for any purpose and without fee is hereby
  granted, provided that the above copyright notice appear in all
  copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#include <lpc17.h>

/* Reset_Handler variables defined in linker script */
extern unsigned long _interrupt_vector_table;
extern unsigned long _data;
extern unsigned long _edata;
extern unsigned long _etext;
extern unsigned long _bss;
extern unsigned long _ebss;

extern void __libc_init_array(void);
extern int main(void);

/* Reset Handler */
void Reset_Handler(void)
{
    unsigned long *src, *dest;

	// Copy the data segment initializers from flash to SRAM
	src = &_etext;
	for(dest = &_data; dest < &_edata; )
	{
		*dest++ = *src++;
	}

	// Initialize the .bss segment of memory to zeros
	src = &_bss;
	while (src < &_ebss)
	{
		*src++ = 0;
	}

    __libc_init_array();
    
    // Set the vector table location.
    SCB_VTOR = &_interrupt_vector_table;
    
	main();

	// In case main() fails, have something to breakpoint
	while (1) {;}
}
