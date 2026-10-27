#include <stdint.h>
#include <stddef.h>

#include <lib/kprintf.h>
#include <lib/string.h>

#include <mm/heap.h>

#include <net/net.h>
#include <net/byteorder.h>
#include <net/eth.h>
#include <net/arp.h>
#include <net/ip4.h>

/* Ethernet device */
net_dev_t* eth_dev; 

/* Mask  lower 16 bits of Ethertype to find the correct handler, 
 * so max 16 entires */
#define ETHER_TYPES	0xF
#define ETHER_MASK 	0xF

/* Interface for Ethertype Handler */
typedef struct eth_type_handler_t {
	void (*handler)(pkt_buff* pkt);
} eth_type_handler;

/* Future TODO: Based on the ETHER_TYPE mask, there may be collisions. Linux
 * points to a struct that calls exact protocol based on exact ethertype. */
static const eth_type_handler handlers[ETHER_TYPES] = {
	[0x0] = {
		.handler = ip4_recv,
	},
	[0x6] = {
		.handler = arp_recv,
	},
};

/* Receive an ethernet frame from the layer below */
static void eth_recv(pkt_buff* pkt) {
	/* Cast and call correct protocol based on Ethertype */
	eth_frame* frame = (eth_frame*) pkt -> data;
	uint16_t ether_type = ntohs(frame -> type);

	/* Set the MAC address of the Sender. */
	memcpy(pkt -> MAC, frame -> src, sizeof(pkt -> MAC));

	/* Move the data to the next location */
	pkt_buff_pull(pkt, sizeof(eth_frame));

	/* Call L3 receive protocl. */
	handlers[ether_type & ETHER_MASK].handler(pkt);

	// print_eth_info((eth_frame*)pkt -> data);
}

/* Send an ethernet frame */
void eth_send(pkt_buff* pkt) {
	/* Malloc an eth frame, memcpy, and send off. */
	pkt_buff_push(pkt, sizeof(eth_frame));
	eth_frame* frame = (eth_frame*) pkt -> data;
	
	/* Increment the size field by the size of ethernet frame */
	pkt -> size += sizeof(eth_frame);

	/* Build the Ethernet frame */
	memcpy(frame -> dest, pkt -> MAC, sizeof(pkt -> MAC));
	memcpy(frame -> src, get_mac(), sizeof(pkt -> MAC));
	frame -> type = htons(pkt -> n_type);

	/* Send the frame to DD */
	eth_dev -> send(pkt);
}

/* Register a device driver to Ethernet. */
void register_net_dev(net_dev_t* net_dev) {
	eth_dev = net_dev;
	eth_dev -> recv = eth_recv;
}

/* Print the ethernet frame info*/
void print_eth_info(eth_frame* eth) {
	/* Print destination MAC address. */
	kprintf("Dest: ");
	for(size_t dst = 0; dst < 6; dst++) {
		kprintf("%x:", *((eth -> dest) + dst));
	}
	
	/* Print source MAC address. */
	kprintf("\nSrc: ");
	for(size_t src = 0; src < 6; src++) {
		kprintf("%x:", *((eth -> src) + src));
	}

	/* Print EtherType */
	kprintf("\nEtherType: %x\n", ntohs(eth -> type));
}

uint8_t* get_mac() {
	return eth_dev -> MAC;
}
