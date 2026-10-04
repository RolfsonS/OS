#include <stddef.h>
#include <stdint.h>

#include <drivers/pci.h>

#include <lib/kprintf.h>

#include <arch/i386/io.h>

#include <mm/heap.h>

/* Pointer to PCI enumerated PCI device info on kernel heap. */
pci_device0_t* pci_device0_info = NULL;




/* IO Port Read and Writes, as well as simple bit shifting operations for extracting fields.
 * Low level Helper Section. Brief description provided for each.
 */

/* Returns a register value from a PCI device */
static uint32_t pci_reg_read(uint8_t bus, uint8_t device, uint8_t func, uint8_t reg) {
	uint32_t address;
	
	/* Bit shifts */
	address = ((uint32_t)bus) << 16;
	address |= ((uint32_t)device) << 11;
	address |= ((uint32_t)func) << 8;
	address |= ((uint32_t)reg) | 0x80000000;
	
	/* Load IO port with PCI information, receive data back */
	outl(CONFIG_ADDRESS, address);
	return inl(CONFIG_DATA);
}

/* Writes a value to a PCI register */
static void pci_reg_write(uint8_t bus, uint8_t device, uint8_t func, uint8_t reg, uint32_t data) {
	uint32_t address;
	
	/* Bit shifts */
	address = ((uint32_t)bus) << 16;
	address |= ((uint32_t)device) << 11;
	address |= ((uint32_t)func) << 8;
	address |= ((uint32_t)reg) | 0x80000000;
	
	outl(CONFIG_ADDRESS, address);
	outl(CONFIG_DATA, data);
}

/* Return device ID */
static uint16_t pci_device_id(uint32_t pci_reg) {
	return (uint16_t)(pci_reg >> 16);
}

/* Return Vendor ID */
static uint16_t pci_vendor_id(uint32_t pci_reg) {
	return (uint16_t)pci_reg;
}

/* Return PCI class code */
static uint8_t pci_class_code(uint32_t pci_reg) {
	return (uint8_t)(pci_reg >> 24);
}

/* Return PCI subclass */
static uint8_t pci_subclass(uint32_t pci_reg) {
	return (uint8_t)(pci_reg >> 16);
}

/* Return PCI Prog IF */
static uint8_t pci_prog_if(uint32_t pci_reg) {
	return (uint8_t)(pci_reg >> 8);
}

/* Return PCI revision ID */
static uint8_t pci_revision_id(uint32_t pci_reg) {
	return (uint8_t) pci_reg;
}




/* Mid level logic. Involves correctly intialization PCI type 0 devices.\
 * Read bottom to top.
 */

/* Helper function for print_dev_info, used to output BAR information */
static void print_bars(pci_device0_t* pci_dev) {
	/* PCI Type 0 devices have 6 base address registers. */
	for(size_t bar = 0; bar < 6; bar++) {
		/* Using pci_dev -> bars[bar]. instead of double ->, as seen above. */
		if(pci_dev -> bars[bar].used == 0x0) {
			// kprintf("BAR not in use\n");
			continue;
		} else if(pci_dev -> bars[bar].memory)  {
			uint32_t addr	 = pci_dev -> bars[bar].addr;
			uint8_t type 	 = pci_dev -> bars[bar].type;
			uint8_t prefetch = pci_dev -> bars[bar].prefetch;
			uint32_t size 	 = pci_dev -> bars[bar].size;
			kprintf("ADDR: %x, TYPE: %x, PREFETCH: %x, SIZE: %x\n", addr, type, prefetch, size);
		} else {
			uint32_t addr 	= pci_dev -> bars[bar].addr;
			uint32_t size 	= pci_dev -> bars[bar].size; 
			kprintf("ADDR: %x, SIZE: %x \n", addr, size);
		}
	}
}

/* Obtain the size of the BAR space. */
static void config_addr_space(pci_device0_t* pci_dev, uint8_t bar) {
	if(!(pci_dev -> bars[bar].used)) {
		return;
	}

	/* To determine the amount of address spave needed by a PCI device,
	 * write a value of all 1's to BAR, then read it back.
	 */
	uint8_t reg = 0x10 + (0x4 * bar);
	uint32_t val = 0xFFFFFFFF;
	pci_reg_write(pci_dev -> bus, pci_dev -> device, pci_dev -> function, reg, val);
	uint32_t addr_space = pci_reg_read(pci_dev -> bus, pci_dev -> device, pci_dev -> function, reg);

	/* Depending on BAR type, MMIO or Port Based IO, apply mask, extract */
	if(pci_dev -> bars[bar].memory) {
		addr_space &= 0xFFFFFFF0;
	} else {
		addr_space &= 0xFFFFFFFC;
	}
	addr_space = ~addr_space;
	addr_space += 0x1;

	/* Write this to correct BAR, and restore original BAR config space */
	pci_dev -> bars[bar].size = addr_space;
	uint32_t original_bar = 0;
	if (pci_dev -> bars[bar].memory) {
		original_bar = pci_dev -> bars[bar].addr |
			       (uint32_t)(pci_dev -> bars[bar].prefetch << 3) |
			       (uint32_t)(pci_dev -> bars[bar].type << 2);
	} else {
		original_bar = pci_dev -> bars[bar].addr;
	}
	pci_reg_write(pci_dev -> bus, pci_dev -> device, pci_dev -> function, reg, original_bar);
}

/* Enable reading access to BAR. More "disabling" brief MMIO request when size exist in register,
 * not the correct address. */
static void enable_addr_read(pci_device0_t* pci_dev) {
	/* As reading addresses from bars requires these bits, we just wrap that behavior in this function.
	 * So, we must read reg, modify 2 lowest bits, and write back.
	 */
	uint32_t reg_val = pci_reg_read(pci_dev -> bus, pci_dev -> device, pci_dev -> function, 0x4);
	uint32_t reg_dis = reg_val ^ 0x3;
	pci_reg_write(pci_dev -> bus, pci_dev -> device, pci_dev -> function, 0x4, reg_dis);

	/* Now, we are good to Read/Write to BARs without issue. 
	 */
	for(uint8_t bar = 0; bar < 6; bar++) {
		config_addr_space(pci_dev, bar);	
	}
	
	/* Restore Command Reg */
	pci_reg_write(pci_dev -> bus, pci_dev -> device, pci_dev -> function, 0x4, reg_val);
}

/* Configure information regarding a Base Address Register. */
static void config_bars(uint8_t bus, uint8_t device, uint8_t func, pci_device0_t* pci_dev) {
	for(size_t bar = 0; bar < 6; bar++) {
		uint8_t offset = 0x10 + (0x4 * bar);
		uint32_t read_bar = pci_reg_read(bus, device, func, offset);
		if(read_bar == 0x0) {
			(pci_dev -> bars + bar) -> used 	= 0x0; 
		} else if(!(read_bar & 0x1)) {
			(pci_dev -> bars + bar) -> used 	= 0x1; 
			(pci_dev -> bars + bar) -> addr 	= read_bar & 0xFFFFFFF0;
			(pci_dev -> bars + bar) -> memory 	= 0x1;
			(pci_dev -> bars + bar) -> type		= (read_bar >> 0x1) & 0x3;
			(pci_dev -> bars + bar) -> prefetch	= (read_bar >> 0x3) & 0x1; 
		} else {
			(pci_dev -> bars + bar) -> used 	= 0x1;
			(pci_dev -> bars + bar) -> addr 	= read_bar & 0xFFFFFFFC;
			(pci_dev -> bars + bar) -> memory 	= 0x0;
		}
	}
}

/* Add PCI device info to a PCI type 0 struct. */
static void add_dev_info(uint8_t bus, uint8_t device, uint8_t func, pci_device0_t* pci_dev) {
	/* All relevant register reads. */
	uint32_t reg_0x0	= pci_reg_read(bus, device, func, 0x0);
	uint32_t reg_0x8	= pci_reg_read(bus, device, func, 0x8);

	/* All relevant fields extracted from register reads. */
	uint16_t dev_id 	= pci_device_id(reg_0x0);
	uint16_t ven_id		= pci_vendor_id(reg_0x0);

	uint8_t class_code 	= pci_class_code(reg_0x8);
	uint8_t subclass	= pci_subclass(reg_0x8);
	uint8_t prog_if		= pci_prog_if(reg_0x8);
	uint8_t revision_id	= pci_revision_id(reg_0x8);

	/* Relevant struct assignments */
	pci_dev -> device 	= device;
	pci_dev -> bus 		= bus;
	pci_dev -> function 	= func;
	
	pci_dev -> device_id 	= dev_id;
	pci_dev -> vendor_id 	= ven_id;

	pci_dev -> class_code 	= class_code;
	pci_dev -> subclass	= subclass;
	pci_dev -> prog_if	= prog_if;
	pci_dev -> revision_id 	= revision_id;

	/* Configure the base address registers of the device */
	config_bars(bus, device, func, pci_dev);
	/* Extract Size --> Must enable Command Reg first */
	enable_addr_read(pci_dev);
}

/* Add and configure (via helpers), a device to the LL of devices */
static void add_dev(uint8_t bus, uint8_t device, uint8_t func) {
	/* Check whether or not the global pointer to heap data structure is intialized.
	 * If not NULL, we know we can walk the list.
	 */
	if(!pci_device0_info) {
		pci_device0_info = (pci_device0_t*) kmalloc(sizeof(pci_device0_t)); 

		/* Initialize the struct, set the next struct to NULL */
		add_dev_info(bus, device, func, pci_device0_info);
		pci_device0_info -> next = NULL;
	} else {
		/* Do to naming, check the first pointer */
		pci_device0_t* next_dev = pci_device0_info -> next;
		if (!next_dev) {
			pci_device0_info -> next = (pci_device0_t*) kmalloc(sizeof(pci_device0_t));
			add_dev_info(bus, device, func, pci_device0_info -> next);
			pci_device0_info -> next -> next = NULL;
		} else {
			/* More general, iterative case */
			next_dev = pci_device0_info -> next;
			while(next_dev -> next) {
				next_dev = next_dev -> next;	
			}
			next_dev -> next = (pci_device0_t*) kmalloc(sizeof(pci_device0_t));
			add_dev_info(bus, device, func, next_dev -> next);
			next_dev -> next -> next = NULL;
		}
	} 
}

/* Enumerate a devices functions */
static void enumerate_device(uint8_t bus, uint8_t device) {
	/* Function number field is three bits, 8 functions max on each device. */
	for(size_t func = 0; func < 8; func++) {
		uint16_t ven_id = pci_vendor_id(pci_reg_read(bus, device, func, 0)); 
		if(ven_id != 0xFFFF) {
			add_dev(bus, device, func);
		}
	}
}

/* Enumerate the devices on a bus */
static void enumerate_bus(uint8_t bus) {
	/* Device number field is 5 bits, 32 devices max on each bus */
	for(size_t device = 0; device < 32; device++) {
		enumerate_device(bus, device);
	}
}

/* Enumerate PCI devices */
static void brute_force_pci() {
	/* 8 Bits for bus field, maximum of 256 buses */
	for(size_t bus = 0; bus < 256; bus++) {
		enumerate_bus(bus);
	}
}



/* Functions availabe via pci.h. Publically accessible. */
void init_pci() {
	/* Brute forcing PCI space sets up PCI device list and certain BAR information.
	 */
	brute_force_pci();
}

void en_bus_master(pci_device0_t* pci_dev) {
	/* Enable Bus Mastering for a PCI Device */
	uint32_t reg_val = pci_reg_read(pci_dev -> bus, pci_dev -> device, pci_dev -> function, 0x4);
	reg_val ^= 0x4;
	pci_reg_write(pci_dev -> bus, pci_dev -> device, pci_dev -> function, 0x4, reg_val);
}

void dis_bus_master(pci_device0_t* pci_dev) {
	/* Disable Bus Mastering for a PCI Device */
	uint32_t reg_val = pci_reg_read(pci_dev -> bus, pci_dev -> device, pci_dev -> function, 0x4);
	reg_val ^= 0x4;
	pci_reg_write(pci_dev -> bus, pci_dev -> device, pci_dev -> function, 0x4, reg_val);
}

void print_dev_info(pci_device0_t* pci_dev) {
	kprintf("Bus: %x, Device: %x, Function: %x\n", pci_dev -> bus, pci_dev -> device, pci_dev -> function);
	kprintf("Dev_ID: %x, Vendor: %x \n", pci_dev -> device_id, pci_dev -> vendor_id);
	kprintf("CC: %x, SC: %x, PROGIF: %x, REV: %x \n", pci_dev -> class_code, pci_dev -> subclass, pci_dev -> prog_if, pci_dev -> revision_id);

	/* Print BAR info for the device aswell */
	print_bars(pci_dev);
	kprintf("\n");
}
