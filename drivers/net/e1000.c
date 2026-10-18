#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <drivers/e1000.h>
#include <drivers/pci.h>

#include <arch/i386/vmm.h>

#include <net/byteorder.h>
#include <net/net.h>
#include <net/eth.h>

#include <lib/kprintf.h>
#include <lib/string.h>

#include <mm/pmm.h>
#include <mm/heap.h>


/* Values specifying th size of the receive buffer and how many descriptors we use in the ring.
 * It should be noted that 4096 B was choosen so we can easily allocate a slab of physical memory
 */
#define RX_DESCRIPTOR_BUFFER_SIZE 	0x1000
#define RX_DESCRIPTORS 			0x8
#define TX_DESCRIPTOR_BUFFER_SIZE 	0x1000
#define TX_DESCRIPTORS			0x8

/* Redefinition of known virtual address (See vmm.c). 
 * Allows us to alter content of known RAM memory, used for accessing buffers. */
#define kvaddr 0xC03FE000

/* Forward definition for drivers. Needed for struct definition. */
static void e1000_probe(pci_device0_t* pci_dev);
static void e1000_send(void* data, size_t size);

/* MAC address */
uint8_t MAC[6];

/* E1000 Driver as a PCI Driver */
pci_driver_t e1000_driver = {
	.name 	= "e1000",
	.info 	= {0x100e, 0x8086},
	.probe 	= e1000_probe, 
};

/* E1000 driver as a network device. */ 
net_dev_t net_e1000 = {
	.send  	= e1000_send,
};
static net_dev_t* net_dev = &net_e1000;

/* RX Head and Tail values. */
static uint32_t rx_head = 0x0;
static uint32_t rx_tail = RX_DESCRIPTORS - 1;

/* TX Tail value (Head is tracked by hardware) */
static uint32_t tx_tail = 0x0;

/* Virtual MMIO Address */ 
static uint32_t MMIO_LOC = 0xD000000;

/* Struct for a receive descriptor. This expects a 64-bit address,
 * we just set the high bits to 0x0. */
typedef struct rx_descriptor_s {
	uint32_t addr_low;
	uint32_t addr_high;
	uint16_t length; 
	uint16_t checksum;
	uint8_t status;
	uint8_t error;
	uint16_t special;
} __attribute__((packed)) rx_descriptor_t; 

/* Struct for transmit descriptor (Legacy). */
typedef struct tx_descriptor_s {
	uint32_t addr_low;
	uint32_t addr_high;
	uint16_t length;
	uint8_t cso;
	uint8_t cmd;
	uint8_t sta;
	uint8_t css;
	uint16_t special;
} tx_descriptor_t;

rx_descriptor_t* rx_ring = NULL;
tx_descriptor_t* tx_ring = NULL;

/* Correctly set up MMIO paging. 
 * Page permissions are set read/write, present, and not cacheable.
 * You can imagine the problems a cached DMA line would cause. */
static void set_MMIO(pci_device0_t* pci_dev) {
	massive_map_page(MMIO_LOC, pci_dev -> bars[0].addr, P | RW | NC, pci_dev -> bars[0].size);
}

/* Write to NIC register */
static void write_reg(uint32_t reg, uint32_t val) {
	*(volatile uint32_t *)(MMIO_LOC + reg) = val;
}

/* Read from NIC register */
static uint32_t read_reg(uint32_t reg) {
	return *(volatile uint32_t *)(MMIO_LOC + reg);
} 

/* Read the MAC address from RAL and RAH */
static void set_MAC_addr(void) {
	uint32_t MAC_l		= read_reg(RAL);
	uint16_t MAC_h		= (uint16_t) read_reg(RAH);

	memcpy(MAC + 4, &MAC_h, sizeof(uint16_t));
	memcpy(MAC, &MAC_l, sizeof(uint32_t));

	/* MAC address to outward facing net dev aswell */
	memcpy(&net_e1000, MAC, sizeof(MAC));
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

/* Intialization of receive descriptors and relevant registers */
static void init_receive_descriptors(void) {
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
		curr_descriptor -> special = 0x00;
		curr_descriptor -> length = 0x0;
		curr_descriptor -> checksum = 0x0;
		curr_descriptor -> error = 0x0;
		curr_descriptor -> status = 0x0;
	}
}

/* Intialization of transmit descriptors and relevant registers */
static void init_transmit_descriptors(void) {
	/* This is cheating to get a 16-byte aligned address */
	uint32_t mem = (uint32_t) kmalloc(0x1);
	while ((mem & 0xF) != 0xF) mem = (uint32_t) kmalloc(0x1); 
	
	/* Allocate heap memory to the tx_ring. */
	tx_ring = (tx_descriptor_t*) kmalloc(TX_DESCRIPTORS * sizeof(tx_descriptor_t));

	/* Iterate through all of the descriptors we want to create*/
	for (size_t descriptor = 0; descriptor < TX_DESCRIPTORS; descriptor++) {
		tx_descriptor_t* curr_descriptor = tx_ring + descriptor;

		/* Set High Address to 0, and Low Address to a physical memory location */
		curr_descriptor -> addr_high = 0x0;
		curr_descriptor -> addr_low  = (uint32_t) allocate_page();

		/* Reset all other fields */
		curr_descriptor -> special = 0x00;
		curr_descriptor -> length = 0x0;
		curr_descriptor -> css = 0x0;

		/* We actually set the status but to 1. This allows us to use this bit as information
		 * regarding whether or not the current descriptor has been read and DMAd to NIC. */
		curr_descriptor -> sta = 0x1;
		curr_descriptor -> cmd = 0x0;
		curr_descriptor -> cso = 0x0;
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
	
	/* Initialize descriptors */
	init_receive_descriptors();

	/* Set relevant Receive Registers.
	 * First, we set Receive Descriptor Mem Loc Regs */
	uint32_t rd_phy_addr = walk((uint32_t) rx_ring);
	write_reg(RDBAH, 0x0);
	write_reg(RDBAL, rd_phy_addr);
	
	/* Set Receive Decscriptor Length in bytes! */
	write_reg(RDLEN, RX_DESCRIPTORS * sizeof(rx_descriptor_t));

	/* Init Receive Descriptor head and tail. */
	write_reg(RDH, 0x0);
	write_reg(RDT, RX_DESCRIPTORS - 1);

	/* Init Receive Control Register */
	uint32_t receive_ctrl = read_reg(RCTL);

	/* Enable normal operation, no long packets, no loopback. */
	receive_ctrl |= RCTL_EN;
	receive_ctrl &= ((~RCTL_LPE) | (~RCTL_LMB));

	/* Free buffer 1/4 of RDLEN, leave default multicast. */
	receive_ctrl |= RCTL_RDMTS_L;
	receive_ctrl &= ~RCTL_RDMTS_H;
	receive_ctrl &= ~RCTL_MO;

	/* Enable broadcast accept, and configure receive buffer size. */
	receive_ctrl |= RCTL_BAM;
	receive_ctrl |= RCTL_BSIZE_L | RCTL_BSIZE_H | RCTL_BSEX;
	
	/* Stip Ethernet CRC */
	receive_ctrl |= RCTL_SECRC;

	/* Write value back. */
	write_reg(RCTL, receive_ctrl);
}

/* Set up receive components of the NIC. */
static void init_transmit(void) {
	/* Initialize the ring */
	init_transmit_descriptors();
	
	/* Load High and Low base address registers */
	uint32_t td_phy_addr = walk((uint32_t) tx_ring);
	write_reg(TDBAH, 0x0);
	write_reg(TDBAL, td_phy_addr);
	
	/* Load the size of the tx ring */
	write_reg(TDLEN, TX_DESCRIPTORS * sizeof(tx_descriptor_t));

	/* Reset transmit descriptor head and tail. */
	write_reg(TDH, 0x0);
	write_reg(TDT, 0x0);

	/* Set relevant transmit control registers. 
	 * First, enable and pad short packets */
	uint32_t transmit_ctrl = read_reg(TCTL);
	transmit_ctrl |= TCTL_EN | TCTL_PSP;

	/* Set collision threshold and distance, although they are not used
	 * in full duplex */
	transmit_ctrl |= TCTL_CT | TCTL_COLD;

	/* Write value back to reg */
	write_reg(TCTL, transmit_ctrl);
}

/* Set up the relevant interrupts that NIC device should send to CPU */
static void init_interrupts(void) {
	/* To avoid race conditions, set/clearing interrupt causes is done 
	 * via write to two registers. So we first clear everything, then set 
	 * the interrupt causes we actually want to listen for.
	 * Also, read ICR to clear any existing interrupts causes. */
	read_reg(ICR);
	write_reg(IMS, IMS_LSC | IMS_RXDMT0 | IMS_RX0 | IMS_RTX0); 
}

/* Commented out for now */
/* Write a fake interrupt 
static void fake_interrupt(uint32_t interrupt) {
	write_reg(ICS, interrupt);
}

 Set Unicast and Multicast Promiscuous modes 
static void init_debug() {
	uint32_t receive = read_reg(RCTL);
	receive |= RCTL_MPE | RCTL_UPE;
	write_reg(RCTL, receive);
}
*/

static void e1000_probe(pci_device0_t* pci_dev) {	
	/* Initialize the device */
	set_MMIO(pci_dev);

	/* No Interrupts. Not Even Once */
	write_reg(IMC, 0xFFFFFFFF);

	/* Set the MAC address, initialize descriptors */
	set_MAC_addr();
	init_control();
	init_transmit();
	init_receive();

	/* Used for debugging, allows us to see everything on wire.
	 * Example: "HELLO" Packet passed up through network stack. */
	// init_debug();

	/* Allow PCI device to master bus and write interrupts. */
	en_ints(pci_dev);
	en_bus_master(pci_dev);

	/* Register as a network device, and allow for interrupts */
	register_net_dev(&net_e1000);
	init_interrupts();

	// fake_interrupt(IMS_RXDMT0);
}

/* Clear all the lower fields of the rx descriptor. */
static void clear_rx(rx_descriptor_t* rx_desc) {
	rx_desc -> special = 0x00;
	rx_desc -> length = 0x0;
	rx_desc -> checksum = 0x0;
	rx_desc -> error = 0x0;
	rx_desc -> status = 0x0;
}

/* When threading is added, there needs to be a seperation of concerns here. 
 * Currently, this + everything inside the networking stack will run inside ISR.
 * Probably not the best idea... */

/* Poll the rx descriptor ring for packets after an interrupt. */
static void rx_poll(void) {
	/* Essentially, poll the head descriptor, plus any others that may have a packet
	 * waiting in their buffer. For each packet, kmalloc a new address to pass to ethernet layer,
	 * copy over the contents, clear the buffer.
	 *
	 * Also, move the tail behind head after. */
	rx_descriptor_t* head = rx_ring + rx_head;
	uint8_t status = head -> status;
	uint8_t* known = (uint8_t*) kvaddr;

	/* While the current descriptor is done and end of packet, keep reading and sending up contents. */
	while(status & (RDESC_DD | RDESC_EOP)) {
		/* Extract the address, map it to a known address, and kmalloc new memory. */
		uint32_t buff_addr = head -> addr_low;
		map_known(buff_addr);
		uint8_t* new_buff = (uint8_t*) kmalloc(head -> length);
		
		/* Copy contents of memory from rx buffer to new buffer.*/
		memcpy(new_buff, known, head -> length);

		/* Increment tail and set the receive descriptor tail accordingly. */
		rx_tail = (rx_tail + 1) % RX_DESCRIPTORS;
		write_reg(RDT, rx_tail);

		/* Send up to ethernet layer. */
		net_dev -> recv(new_buff, head -> length);

		/* Clear the rx buffer and the rx status fields */
		memset(known, 0x0, head -> length);
		clear_rx(head);

		/* Increment rx_descriptor, update status, and increment global copy of rx_head. */
		head++;
		status = head -> status;
		rx_head = (rx_head + 1) % RX_DESCRIPTORS;
	}
}

/* Send packets to the rx descriptors. */
static void e1000_send(void* data, size_t size) {
	/* Upon initialization of all rx descriptors, we set the DD bit in status to 1.
	 * This is the same process the NIC writing back to descriptor after it has DMAed a 
	 * tx buffer. We set this back to 0 once we write something new to buffer. */ 
	tx_descriptor_t* tail = tx_ring + tx_tail;
	uint8_t status = tail -> sta;
	uint8_t* known = (uint8_t*) kvaddr;
	
	/* The buffer is ok to use, as it has been read. */
	if (status & TDESC_DD) {
		uint32_t buff_addr = tail -> addr_low;

		/* Map tails physical address to known virtual address, and copy the contents
		 * of the memory over to this address. */
		map_known(buff_addr);
		memcpy(known, data, size);

		/* Set the size of the descriptor field. */
		tail -> length = size;

		/* Set relevant transmit descriptor command field bits. Also set DD to 0. */
		tail -> cmd = TDESC_EOP | TDESC_IFCS | TDESC_RS;
		tail -> sta = 0x0;
		
		/* Increment tail and write this value to transmit descriptor tail */
		tx_tail = (tx_tail + 1) % TX_DESCRIPTORS;
		write_reg(TDT, tx_tail);
	} else {
		/* In future, return a error */
	}
}



/* Based on the value read from Interrupt cause, process accordingly. */
void handle_interrupt(void) {
	uint32_t cause = read_reg(ICR);
	
	if(cause & IMS_LSC) { 
	//	kprintf("LSC \n");
	}

	if(cause & IMS_RXDMT0) {
	//	kprintf("MT0 \n");
		rx_poll();
	}

	if(cause & IMS_RX0) {
	//	kprintf("RX0 \n");
		rx_poll();
	}

	if(cause & IMS_RTX0) {
	//	kprintf("RTX0 \n");
		rx_poll();
	}

	// kprintf("Cause: %x\n", cause);
}













