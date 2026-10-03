#include <stdint.h>
#include <stddef.h>
#include "idt_handlers.h"
#include "terminal.h"
#include "pic.h"
#include "io.h"
#include "kprintf.h"


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
