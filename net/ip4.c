#include <stdint.h>
#include <stddef.h>

#include <lib/kprintf.h>

#include <net/ip4.h>
#include <net/eth.h>
#include <net/byteorder.h>

/* Statically defined, global IP address */
static const uint8_t ip4_addr[4] = {192, 168, 100, 2}; 

void ip4_recv(void* data) {
	kprintf("TEST IPv4 \n");
}

uint8_t* ret_ip(void) {
	return ip4_addr;
}
