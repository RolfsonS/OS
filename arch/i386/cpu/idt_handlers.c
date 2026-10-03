#include <stdint.h>
#include <stddef.h>

#include <arch/i386/idt_handlers.h>
#include <arch/i386/io.h>
#include <arch/i386/pic.h>

#include <drivers/terminal.h>

#include <lib/kprintf.h>


void general_idt_handler(void){
	/* Do nothing */
	kprintf("Page Fault");
}

void irq_0_handler(void) {
	/* Send end of interrupt command - Pin 0 */
	pic_sendEOI(0);
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
