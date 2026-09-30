#ifndef IDT_H
#define IDT_H

/* Struct stored in IDTR register.
 * Size indicates the size of the idt, offset indicates the location in memory. */
struct __attribute__((packed)) idt {
	uint16_t size;
	uint32_t offset;
};

/* Creates a IDT descriptor by manipulating the necessary field.
 * See https://wiki.osdev.org/Interrupt_Descriptor_Table for more.
 *
 * Param: uint32_t offset, 32-bit value representing entry point of ISR
 * Param: uint16_t selector, 16-bit Segment selector pointing to a valid code segment in GDT
 * Param: uint8_t info_byte, 8-bit indicating gate type, CPU privilege, and present bit
 *
 * Return: uint64_t, a 64 bit descriptor table entry
 */ 
uint64_t create_idt_descriptor(uint32_t offset, uint16_t selector, uint8_t info_byte);

/* Set up the IDT descriptor entries. */
void setup_idt_descriptors(void);

#endif
