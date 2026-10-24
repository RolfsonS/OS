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

/* Receive an ethernet frame from the layer below */
static void eth_recv(pkt_buff* pkt) {
	/* Cast and call correct protocol based on Ethertype */
	eth_frame* frame = (eth_frame*) pkt -> data;
	uint16_t ether_type = ntohs(frame -> type);

	/* Set the MAC address of the Sender. */
	pkt -> MAC = frame -> source;

	/* TODO: Change this to an array based call */
	if(ether_type == ARP_ETH_TYPE ) {
		arp_recv(pkt);
	} else if (ether_type == IP4_ETH_TYPE) {
	//	ip4_recv(frame -> payload);
	}

	print_eth_info((eth_frame*)pkt -> data);
}

/* Send an ethernet frame */
void eth_send(void* dest_mac, void* packet, size_t size, uint16_t type) {
	/* Malloc an eth frame, memcpy, and send off. */
	eth_frame* frame = (eth_frame*) kmalloc(size + sizeof(eth_frame));
	
	memcpy(frame -> dest, dest_mac, sizeof(frame -> dest));
	memcpy(frame -> src, get_mac, sizeof(frame -> src));
	frame -> type = htons(type);
	memcpy(frame -> payload, packet, size);

	eth_dev -> send(frame, size + sizeof(eth_frame));
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
