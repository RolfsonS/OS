#include <stdint.h>
#include <stddef.h>

#include <lib/kprintf.h>

#include <net/net.h>
#include <net/byteorder.h>
#include <net/eth.h>

/* Ethernet device */
net_dev_t* eth_dev; 

static void eth_recv(void* data, size_t size) {
	uint8_t* test = (uint8_t*) data;

	for(size_t i = 0; i < size; i++) {
		kprintf("%c ", *(test + i));
	}
}


void register_net_dev(net_dev_t* net_dev) {
	eth_dev = net_dev;
	eth_dev -> recv = eth_recv;
}

void print_eth_info() {
	// eth_dev -> recv();
	// eth_dev -> send();

	for(size_t i = 0; i < 6; i++) {
		kprintf("%x :", eth_dev -> MAC[i]);
	}
}
