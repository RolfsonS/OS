#ifndef GDT_H
#define GDT_H

/* Struct stored in GDTR register.
 * Size indicates the size of the gdt, offset indicates location in memory. */
struct __attribute__((packed)) gdt {
	uint16_t size;
	uint32_t offset;
};

/* Creates a GDT descriptor by manipulating necessary fields. 
 * See https://wiki.osdev.org/Global_Descriptor_Table for more.
 *
 * Param: uint32_t base, 32-bit value containting the linear address where segment begins
 * Param: uint32_t limit, 20-bit value giving the maximum addressable unit
 * Param: uint8_t flags, 4-bit value indicating granularity, size, and long mode
 * Param: uint8_t access, 8-bit value indicating P, DPL, S, E, DC, RW, and A flag bits 
 *
 * Return: uint64_t, a 64 bit descriptor table entry
 */
uint64_t create_gdt_descriptor(uint32_t base, uint32_t limit, uint8_t flags, uint8_t access);

/* Set up the GDT descriptor entries. */
void setup_gdt_descriptors(void);

#endif
