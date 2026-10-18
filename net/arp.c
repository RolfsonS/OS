#include <stdint.h>
#include <stddef.h>

#include <lib/kprintf.h>

#include <net/arp.h>
#include <net/byteorder.h>
#include <net/eth.h>

void arp_recv(void* data) {
	kprintf("Hello from ARP\n");
}
