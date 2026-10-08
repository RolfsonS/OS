#ifndef PCI_H
#define PCI_H

#define CONFIG_ADDRESS 	0xCF8
#define CONFIG_DATA	0xCFC

/* Struct representing contents of a Base Address Register (BAR) */
typedef struct bar_s {
	uint32_t addr;
	uint8_t memory;
	uint8_t type;
	uint8_t prefetch;
	uint8_t used;

	uint32_t size;
} bar_t;


/* Struct representing an end device (0x0 header type)*/
typedef struct pci_device0_s {
	/* Discovered during enumeration phase, may be used later for some writes.*/
	uint8_t device, bus, function;

	/* All data read from registers */
	uint16_t device_id, vendor_id;
	uint8_t class_code, subclass, prog_if, revision_id;
	bar_t bars[6];
	
	/* Interrupt Pin and Line Information */
	uint8_t interrupt_pin, interrupt_line;

	/* Points to next PCI device */
	struct pci_device0_s* next;
} pci_device0_t;

/* A general defintion of a PCI device driver, used for probing and loading the correct device driver.
 * For a specific device, this is instantiated in that devices file.
 */
typedef struct pci_driver_s {
	char* name;
	uint16_t info[2];
	void (*probe)(pci_device0_t* pci_dev);
} pci_driver_t;


/* Wrapper for scanning PCI space.
 */
void init_pci();


/* Print relevant fields of struct for a specific device.
 * 
 * Param: pci_device0_t* pci_dev, Device to print info for
 */
void print_dev_info(pci_device0_t* pci_dev);


/* Allows a Device to take control over the PCI Bus (and then disable).
 * Both these used primarily for Drivers 
 */
void en_bus_master(pci_device0_t* pci_dev);

void dis_bus_master(pci_device0_t* pci_dev);
#endif
