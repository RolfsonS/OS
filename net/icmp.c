#include <stdint.h>
#include <stddef.h>

#include <lib/kprintf.h>

#include <net/ip4.h>
#include <net/eth.h>
#include <net/byteorder.h>
#include <net/icmp.h>


static void print_icmp_info(icmp_packet* packet) {
	uint8_t t = packet -> type;
	uint8_t c = packet -> code;
	uint16_t chsm = ntohs(packet -> checksum);
	kprintf("TYPE: %x, CODE: %x, CHKSM: %x\n", t, c, chsm);

}

void icmp_recv(void* data) {
	/* Cast to ICMP packet, then make decision */
	icmp_packet* packet = (icmp_packet*) data;

	print_icmp_info(packet);
}



