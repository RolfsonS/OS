#include <stdint.h>
#include <stddef.h>

#include <drivers/e1000.h>

#include <lib/kprintf.h>

const uint32_t MMIO_BAR = 0x0;


void test() {
	kprintf("Hello\n");
}

void write_reg(uint32_t reg, uint32_t val) {
	*(uint32_t *)(MMIO_BAR + reg) = val;
}

uint32_t read_reg(uint32_t reg) {
	return *(uint32_t *)(MMIO_BAR + reg);
}









