#include <stdint.h>
#include <stddef.h>

#include <drivers/e1000.h>
#include <drivers/pci.h>

#include <lib/kprintf.h>

/* Single forward definition for e1000 driver. Needed for struct definition. */
static void e1000_probe(pci_device0_t* pci_dev);

/* E1000 Driver as a PCI Driver */
pci_driver_t e1000_driver = {
	.name 	= "e1000",
	.info 	= {0x100e, 0x8086},
	.probe 	= e1000_probe, 
};

static uint32_t MMIO_LOC = 0x0;

void test() {
//	kprintf("Hello\n");
}

static void set_MMIO(pci_device0_t* pci_dev) {
	MMIO_LOC = pci_dev -> bars[0].addr;	
}

static void write_reg(uint32_t reg, uint32_t val) {
	*(uint32_t *)(MMIO_LOC + reg) = val;
}

static uint32_t read_reg(uint32_t reg) {
	return *(uint32_t *)(MMIO_LOC + reg);
} 

static void e1000_probe(pci_device0_t* pci_dev) {
	print_dev_info(pci_dev);
	kprintf("Loading e1000 driver... \n");
	set_MMIO(pci_dev);
	kprintf("MMIO: %x \n", MMIO_LOC);
}






