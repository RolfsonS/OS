#include <stdint.h>
#include <stddef.h>
#include "pic.h"
#include "io.h"
#include "idt.h"
#include "terminal.h"

/* Static RAM IDT elements. */
static struct idt interrupt_descriptor_table; 
static uint64_t	interrupt_entries[256];

/* Assembly interrupt handler stubs.
 * Required for pushing GPRs onto stack, 
 */
extern void general_idt();
extern void irq_0();
extern void irq_1();

/* An extern assembly function, need for using LIDT assembly command. */
extern void idt_setup(uint32_t);

uint64_t create_idt_descriptor(uint32_t offset, uint16_t selector, uint8_t info_byte){	
	uint64_t descriptor;

	/* Set the upper 32 bits */
	descriptor  = offset & 0xFFFF0000; 		// Bits 63-48: 31-16 Offset bits
	descriptor |= (info_byte << 8) & 0x0000FF00; 	// Bits 47-40: 7-0 Info bits

	/* Shift decsriptor left 32 bits */
	descriptor <<= 32;

	/* Set the lower 32 bits, and return */
	descriptor |= (selector << 16) & 0xFFFF0000;	// Bits 31-16: 15-0 Segment bits
	descriptor |= offset & 0x0000FFFF;		// Bits 15-0: 15-0 Offset bits

	return descriptor;
}

void setup_idt_descriptors(void) {

	/* Configures Programmable Interrput Controller */	
	configure_pic();	

	/* Set all interrupts to a default handler.
	 * Interrupt 33 - Timer Interrupt.
	 * Interrupt 34 - Keyboard Interrput.
	 */
	for(size_t i = 0; i < 256; i++) {
		interrupt_entries[i] = create_idt_descriptor((uint32_t)general_idt, 0x8, 0x8E);
	}
	interrupt_entries[32] = create_idt_descriptor((uint32_t)irq_0, 0x8, 0x8E);
	interrupt_entries[33] = create_idt_descriptor((uint32_t)irq_1, 0x8, 0x8E);

	/* Global IDT assignment */
	interrupt_descriptor_table.size = sizeof(interrupt_entries) - 1;
	interrupt_descriptor_table.offset = (uint32_t) interrupt_entries;

	/* Call extern assembly for assigning IDTR reigster */
	idt_setup((uint32_t) &interrupt_descriptor_table);
}


