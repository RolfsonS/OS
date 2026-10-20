#include <stdint.h>
#include <stddef.h>

#include <lib/kprintf.h>

#include <net/net.h>
#include <net/byteorder.h>
#include <net/eth.h>
#include <net/arp.h>
#include <net/ip4.h>

/* Ethernet device */
net_dev_t* eth_dev; 

/* Receive an ethernet frame from the layer below */
static void eth_recv(void* data, size_t size) {
	/* Cast and call correct protocol based on Ethertype */
	eth_frame* frame = (eth_frame*) data;
	uint16_t ether_type = ntohs(frame -> type);
	//kprintf("SIZE: %d\n", size);

	if(ether_type == ARP_ETH_TYPE ) {
		arp_recv(frame -> payload);
	} else if (ether_type == IP4_ETH_TYPE) {
		// ip4_recv(frame -> payload);
	}

	//print_eth_info(data);
}

/* Send an ethernet frame */
void eth_send(void* dest_mac, void* packet, size_t size) {
	eth_frame* 	
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
