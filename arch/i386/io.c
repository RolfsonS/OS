#include <stdint.h>

#include <arch/i386/io.h>

void outb(uint16_t port, uint8_t val){
	/* Assembly for writing to an output port */
	__asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

void outw(uint16_t port, uint16_t val) {
	/* Assembly for writing to an output port */
	__asm__ volatile ( "outw %w0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

void outl(uint16_t port, uint32_t val) {
	/* Assembly for writing to an output port */
	__asm__ volatile ( "outl %k0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

uint8_t inb(uint16_t port) {
	uint8_t ret;
	/* Assembly for reading an input port */
	__asm__ volatile ( "inb %w1, %b0"
        	           : "=a"(ret)
                	   : "Nd"(port)
        		   : "memory");
	return ret;
}

uint16_t inw(uint16_t port) {
	uint16_t ret;
	/* Assembly for reading an input port */
	__asm__ volatile ( "inw %w1, %w0"
        	           : "=a"(ret)
                	   : "Nd"(port)
        		   : "memory");
	return ret;
}

uint32_t inl(uint16_t port) {
	uint32_t ret;
	/* Assembly for reading an input port */
	__asm__ volatile ( "inl %w1, %k0"
        	           : "=a"(ret)
                	   : "Nd"(port)
        		   : "memory");
	return ret;
}
