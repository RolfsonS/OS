#include <stdint.h>

#include <arch/i386/gdt.h>

/* Static RAM GDT elements */ 
static struct gdt global_descriptor_table;
static uint64_t entries[5];

/* An extern assembly function, need for using LGDT assembly command.*/
extern void gdt_setup(uint32_t);

uint64_t create_gdt_descriptor(uint32_t base, uint32_t limit, uint8_t flags, uint8_t access){
	uint64_t descriptor;
	
	/* Set the upper 32 bits */
	descriptor  = base & 0xFF000000; 		// Bits 61-56: 31-24 Base bits
	descriptor |= (base >> 16) & 0x000000FF; 	// Bits 39-32: 23-16 Base bits
	descriptor |= (access << 8) & 0x0000FF00; 	// Bits 47-40: 7-0 Access bits
	descriptor |= (flags << 20) & 0x00F00000; 	// Bits 55-52: 3-0 Flag bits
	descriptor |= limit & 0x000F0000; 		// Bits 51-48: 19-16 Limit bits
					  
	/* Shift descriptor left 32 bits */
	descriptor <<= 32;
	
	/* Set the lower 32 bits, and return */
	descriptor |= base << 16; 			// Bits 31-16: 15-0 Base bits
	descriptor |= limit & 0x0000FFFF; 		// Bits 15-0: 15-0 Limit bits
	
	return descriptor; 
}

void setup_gdt_descriptors(void) {	
	
	/* Setting up global descriptor entries.
	 * See https://wiki.osdev.org/GDT_Tutorial for more on layout.
	 */
	entries[0] = create_gdt_descriptor(0, 0, 0, 0);
	entries[1] = create_gdt_descriptor(0, 0xFFFFF, 0xC, 0x9A);
	entries[2] = create_gdt_descriptor(0, 0xFFFFF, 0xC, 0x92);
	entries[3] = create_gdt_descriptor(0, 0xFFFFF, 0xC, 0xFA);
	entries[4] = create_gdt_descriptor(0, 0xFFFFF, 0xC, 0xF2);

	/* Global GDT assignment */
	global_descriptor_table.offset = (uint32_t) entries;
	global_descriptor_table.size = sizeof(entries) - 1;

	/* Call extern assembly for assigning GDTR register*/
	gdt_setup((uint32_t)&global_descriptor_table);
}
