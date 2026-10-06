#include <stdint.h>
#include <stddef.h>

#include <drivers/e1000.h>
#include <drivers/pci.h>

#include <arch/i386/vmm.h>

#include <net/byteorder.h>

#include <lib/kprintf.h>
#include <lib/string.h>

/* Single forward definition for e1000 driver. Needed for struct definition. */
static void e1000_probe(pci_device0_t* pci_dev);

uint8_t MAC[6];

/* E1000 Driver as a PCI Driver */
pci_driver_t e1000_driver = {
	.name 	= "e1000",
	.info 	= {0x100e, 0x8086},
	.probe 	= e1000_probe, 
};

static uint32_t MMIO_LOC = 0xD000000;

void test() {
//	kprintf("Hello\n");
}

/* Correctly set up MMIO paging. 
 * Page permissions are set read/write, present, and not cacheable.
 * You can imagine the problems a cached DMA line would cause. */
static void set_MMIO(pci_device0_t* pci_dev) {
	massive_map_page(MMIO_LOC, pci_dev -> bars[0].addr, P | RW | NC, pci_dev -> bars[0].size);
}

static void write_reg(uint32_t reg, uint32_t val) {
	*(uint32_t *)(MMIO_LOC + reg) = val;
}

static uint32_t read_reg(uint32_t reg) {
	return *(uint32_t *)(MMIO_LOC + reg);
} 

static void set_MAC_addr(void) {
	uint32_t MAC_low 	= ntohl(read_reg(RAL));
	uint16_t MAC_high	= (uint16_t) ntohl(read_reg(RAH));

	kprintf("LOW: %x, HIGH: %x\n", MAC_low, MAC_high);
	
	memcpy(MAC + 4, &MAC_high, sizeof(uint16_t));
	memcpy(MAC, &MAC_low, sizeof(uint32_t));

	for(size_t i = 0; i < 6; i++) {
		kprintf("%x:", MAC[i]);
	}
}

static void e1000_probe(pci_device0_t* pci_dev) {
	print_dev_info(pci_dev);
	kprintf("Loading e1000 driver... \n");
	
	set_MMIO(pci_dev);
	kprintf("MMIO: %x \n", MMIO_LOC);
	
	set_MAC_addr();
}






