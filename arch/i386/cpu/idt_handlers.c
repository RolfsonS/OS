#include <stdint.h>
#include <stddef.h>

#include <arch/i386/idt_handlers.h>
#include <arch/i386/io.h>
#include <arch/i386/pic.h>

#include <drivers/terminal.h>
#include <drivers/e1000.h>

#include <lib/kprintf.h>

#include <kernel/sched.h>


void general_idt_handler(void){
	/* Do nothing */
	kprintf("Page Fault");
}

void irq_0_handler(void) {
	/* Timer  interrupts are used for processor scheduling. *	
	 * We send the EOI to PIC first, before task switching so we don't
	 * run a single process forever. 
	 *
	 * Send end of interrupt command - Pin 0. */
	pic_sendEOI(0);

	schedule();
}

void irq_1_handler(void) {
	/* Read the keyboard character, write to screen */
	char c = inb(0x60);
	char arr[2] = {c, '\0'};

	terminal_setcolor(4);
	terminal_writestring(arr);

	/* Send end of interrupt command - Pin 1 */
	pic_sendEOI(1);
}

void irq_11_handler(void) {
	// kprintf("In IRQ Handler\n");
	handle_interrupt();
	pic_sendEOI(11);
}
