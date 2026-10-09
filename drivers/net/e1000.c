#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <drivers/e1000.h>
#include <drivers/pci.h>

#include <arch/i386/vmm.h>

#include <net/byteorder.h>

#include <lib/kprintf.h>
#include <lib/string.h>

#include <mm/pmm.h>
#include <mm/heap.h>

#define RX_DESCRIPTOR_BUFFER_SIZE 	0x1000
#define RX_DESCRIPTORS 			0x8



/* Single forward definition for e1000 driver. Needed for struct definition. */
static void e1000_probe(pci_device0_t* pci_dev);

/* MAC address */
uint8_t MAC[6];

/* E1000 Driver as a PCI Driver */
pci_driver_t e1000_driver = {
	.name 	= "e1000",
	.info 	= {0x100e, 0x8086},
	.probe 	= e1000_probe, 
};

/* Virtual MMIO Address */ 
static uint32_t MMIO_LOC = 0xD000000;

/* Struct for a receive descriptor. This expects a 64-bit address,
 * we just set the high bits to 0x0. */
typedef struct rx_descriptor_s {
	uint32_t addr_high, addr_low;
	uint16_t special, length, checksum;
	uint8_t error, status;
} rx_descriptor_t;

rx_descriptor_t* rx_ring = NULL;

/* Correctly set up MMIO paging. 
 * Page permissions are set read/write, present, and not cacheable.
 * You can imagine the problems a cached DMA line would cause. */
static void set_MMIO(pci_device0_t* pci_dev) {
	massive_map_page(MMIO_LOC, pci_dev -> bars[0].addr, P | RW | NC, pci_dev -> bars[0].size);
}

/* Write to NIC register */
static void write_reg(uint32_t reg, uint32_t val) {
	*(uint32_t *)(MMIO_LOC + reg) = val;
}

/* Read from NIC register */
static uint32_t read_reg(uint32_t reg) {
	return *(uint32_t *)(MMIO_LOC + reg);
} 

/* Read the MAC address from RAL and RAH */
static void set_MAC_addr(void) {
	uint32_t MAC_l		= read_reg(RAL);
	uint16_t MAC_h		= (uint16_t) read_reg(RAH);

	memcpy(MAC + 4, &MAC_h, sizeof(uint16_t));
	memcpy(MAC, &MAC_l, sizeof(uint32_t));
}

/* Set-up a variety of the NICs control register bits */
static void init_control(void) {
	/* Auto negotiate speed and duplex */
	uint32_t control = read_reg(CTRL);
	control |= CTRL_ASDE | CTRL_SLU;

	/* No PHY Reset, No Invert Active Low Signal.
	 * These default to zero, but write to make sure.*/
	control &= ((~CTRL_PHY_RST) | (~CTRL_ILOS));

	/* Disable VLAN Mode */
	control &= ~CTRL_VME;
	
	/* Disable ALL Flow Control Registers */
	write_reg(FCAL, 0x0);
	write_reg(FCAH, 0x0);
	write_reg(FCT, 0x0);
	write_reg(FCCTV, 0x0);

	/* Write all updated values to the register */
	write_reg(CTRL, control);
}

/* Intialization of receive descriptor and relevant registers */
static void init_receive_descriptors() {
	/* This is cheating to get a 16-byte aligned address */
	uint32_t mem = (uint32_t) kmalloc(0x1);
	while ((mem & 0xF) != 0xF) mem = (uint32_t) kmalloc(0x1); 

	/* Allocate heap memory to the rx_ring. */
	rx_ring = (rx_descriptor_t*) kmalloc(RX_DESCRIPTORS * sizeof(rx_descriptor_t));

	/* Iterate through all of the descriptors we want to create*/
	for (size_t descriptor = 0; descriptor < RX_DESCRIPTORS; descriptor++) {
		rx_descriptor_t* curr_descriptor = rx_ring + descriptor;

		/* Set High Address to 0, and Low Address to a physical memory location */
		curr_descriptor -> addr_high = 0x0;
		curr_descriptor -> addr_low  = (uint32_t) allocate_page();

		/* Reset all other fields */
		curr_descriptor -> special = 0x0;
		curr_descriptor -> length = 0x0;
		curr_descriptor -> checksum = 0x0;
		curr_descriptor -> error = 0x0;
		curr_descriptor -> status = 0x0;
	}
	
	/* Set relevant Receive Registers */

	for (size_t descriptor = 0; descriptor < 1; descriptor++) {
		rx_descriptor_t* c = rx_ring + descriptor;
		kprintf("RX_RNG: %x\n", c);
		uint32_t rx_ring_phy = walk(c);
		kprintf("RX_RNG_PHY: %x\n ", rx_ring_phy);
	}
}

/* Set up receive components of the NIC */
static void init_receive(void) {
	/* For a real peice of NIC hardware, you may have to read the MAC 
	 * address from EEPROM, and write it to RAL/RAH. For this NIC, we 
	 * just read directly from these registers... just a step to be aware of */

	/* Initialize the Multicast Table Array to all 0's (128 4 Byte Registers)*/
	for (size_t reg = 0; reg < 128; reg++) {
		write_reg((MTA + (reg * 0x4)), 0x0);
	}
	
	init_receive_descriptors();
}


static void e1000_probe(pci_device0_t* pci_dev) {
	//print_dev_info(pci_dev);
	//kprintf("Loading e1000 driver... \n");
	
	set_MMIO(pci_dev);
	//kprintf("MMIO: %x \n", MMIO_LOC);
	
	set_MAC_addr();

	init_control();
	
	//kprintf("test");
	init_receive();
	kprintf("test");
}






