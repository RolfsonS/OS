#include <stdint.h>
#include <stddef.h>

#include <lib/kprintf.h>

#include <mm/heap.h>

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

static void icmp_echo_req(void* packet, void* ip_addr) {
	icmp_packet* snd_packet = (icmp_packet*) kmalloc(sizeof(icmp_packet));
	
	/* Set type and code. */
	snd_packet -> type = ECHO_REPLY;
	snd_packet -> code = 0x0;
	
	/* Set checksum to 0, then compute the real checksum */
	snd_packet -> checksum = 0x0;
	snd_packet -> checksum = inet_checksum(packet, sizeof(icmp_packet));

	/* Send this IP stack */
	ip4_send(ip_addr, snd_packet, sizeof(icmp_packet), IP4_ICMP);
}

/* Receive data from IP4 layer */
void icmp_recv(void* data, void* ip_addr) {
	/* Cast to ICMP packet, then make decision */
	icmp_packet* packet = (icmp_packet*) data;

	/* Check the type, pass to the correct layer (also may need to cast payload) */
	uint8_t type = packet -> type;
	if(type == ECHO_REQUEST) {
		/* Send a packet responding to echo request */	
		kprintf("ICMP REPLY");
		icmp_echo_req(packet, ip_addr);
	}
	print_icmp_info(packet);
}

/* Compute the Checksum of a packet */
uint16_t inet_checksum(void* data, size_t size) {
	uint8_t* entry = (uint8_t*) data;
	uint32_t sum = 0;

	/* Ones complement addition */
	while(size > 1) {
		sum += (entry[0] << 8) | entry[1];
		size -= 2;
		entry += 2;
	}

	/* If extra byte, add it with 0 appended */
	if(size) {
		sum += (entry[0] << 8);
	}

	/* Wrap the overflow back in */
	while (sum >> 16) {
		sum = (sum & 0xFFFF) + (sum >> 16);
	}

	return (uint16_t) sum;
}
