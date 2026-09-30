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
} bar_t;


/* Struct representing an end device (0x0 header type)*/
typedef struct pci_device0_s {
	/* Discovered during enumeration phase, may be used later for some writes.*/
	uint8_t device, bus, function;

	/* All data read from registers */
	uint16_t device_id, vendor_id;
	uint8_t class_code, subclass, prog_if, revision_id;
	bar_t bars[6];

	/* Points to next PCI device */
	struct pci_device0_s* next;
} pci_device0_t;

/* Wrapper for scanning PCI space.
 */
void init_pci();

/* Returns a register from a PCI device
 *
 * Param: uint8_t bus, Allow configuration software to chosse a specific PCI bus in the system
 * Param: uint8_t device, Select a specific device on the selected PCI bus
 * Param: uint8_t func, Choose a specific function in a device (if the device supports multiple functions)
 * Param: uint8_t reg, Offset into the 256-byte configuration space of the device
 *
 * Return: uint32_t, Value from selected register on device via I/O ports
 */
uint32_t pci_reg_read(uint8_t bus, uint8_t device, uint8_t func, uint8_t reg);

/* An existing read has been performed, no need to reform bits.
 *
 * Param: uint32_t config, 4 byte config space write
 *
 * Return: uint32_t, Value from selected register on device via I/O ports
 */
uint32_t pci_reg_read32(uint32_t config);

/* Return the device ID for a PCI device 
 *
 * Param: uint32_t pci_reg, a read PCI register
 *
 * Return: uint16_t, Device ID
 */ 
uint16_t pci_device_id(uint32_t pci_reg);

/* Return the Vendor ID for a PCI device 
 *
 * Param: uint32_t pci_reg, a read PCI register
 *
 * Return: uint16_t, Vendor ID
 */
uint16_t pci_vendor_id(uint32_t pci_reg);

/* Return the Class code for a PCI device 
 *
 * Param: uint32_t pci_reg, a read PCI register
 *
 * Return: uint8_t, Class code
 */
uint8_t pci_class_code(uint32_t pci_reg);

/* Return the Subclass for a PCI device 
 *
 * Param: uint32_t pci_reg, a read PCI register
 *
 * Return: uint8_t, Subclass
 */
uint8_t pci_subclass(uint32_t pci_reg);

/* Return the Prog IF for a PCI device 
 *
 * Param: uint32_t pci_reg, a read PCI register
 *
 * Return: uint8_t, Prog IF
 */
uint8_t pci_prog_if(uint32_t pci_reg);

/* Return the Revision ID for a PCI device 
 *
 * Param: uint32_t pci_reg, a read PCI register
 *
 * Return: uint8_t, Revision ID
 */
uint8_t pci_revision_id(uint32_t pci_reg);

/* Brute force scan all possible Buses. Save recursive search for later?
 */
void brute_force_pci();

/* Enumerate all devices on a specific bus
 *
 * Param: uint8_t bus, Bus number to enumerate
 */
void enumerate_bus(uint8_t bus);

/* Enumerate functions on a device. 
 *
 * Param: uint8_t bus, Devices bus 
 * Param: uint8_t device, Device to enumerate functions on 
 */
void enumerate_device(uint8_t bus, uint8_t device);

/* Adds a PCI device to the linked list of known PCI devices
 *
 * Param: uint8_t bus, Bus the device exist on
 * Param: uint8_t device, Device on the bus
 * Param: uint8_t func, Function of device
 */
void add_dev(uint8_t bus, uint8_t device, uint8_t func);

/* Fills in the struct required for the PCI device, a helper function for above
 *
 * Param: uint8_t bus, Bus the device exist on
 * Param: uint8_t device, Device on the bus
 * Param: uint8_t func, Function of device
 * Param: pci_device0_t* pci_dev, Corresponding struct to fill
 */ 
void add_dev_info(uint8_t bus, uint8_t device, uint8_t func, pci_device0_t* pci_dev);

/* Print relevant fields of struct for a specific device.
 * 
 * Param: pci_device0_t* pci_dev, Device to print info for
 */
void print_dev_info(pci_device0_t* pci_dev);


/* Configures the base address registers for a PCI device.
 *
 * Param: uint8_t bus, Bus the device exist on
 * Param: uint8_t device, Device on the bus
 * Param: uint8_t func, Function of device
 * Param: pci_device0_t* pci_dev, Pointer to the PCI device to configure.
 */
void config_bars(uint8_t bus, uint8_t device, uint8_t func, pci_device0_t* pci_dev);

/* Print base address registers for a device.
 *
 * Param: pci_device0_t* pci_dev, Device to print bar info
 */
void print_bars(pci_device0_t* pci_dev);



#endif
