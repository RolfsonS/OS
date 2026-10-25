#include <stdint.h>
#include <stddef.h>

#include <lib/kprintf.h>

#include <mm/heap.h>

#include <net/ip4.h>
#include <net/eth.h>
#include <net/byteorder.h>
#include <net/icmp.h>
#include <net/net.h>

/* Start with 16 ICMP types. */
#define ICMP_TYPES	0xF

static void icmp_echo_req(pkt_buff* pkt);


/* Similar to ETH and IP4, use an array indexed based on ICMP Type. */
typedef struct icmp_handler_t {
	void (*handler) (pkt_buff* pkt);
} icmp_handler;

static const icmp_handler handlers[ICMP_TYPES] = {
	[ECHO_REQUEST] = {
		.handler = icmp_echo_req,
	},
};

static void print_icmp_info(icmp_packet* packet) {
	uint8_t t = packet -> type;
	uint8_t c = packet -> code;
	uint16_t chsm = ntohs(packet -> checksum);
	kprintf("TYPE: %x, CODE: %x, CHKSM: %x\n", t, c, chsm);

}

static void icmp_echo_req(pkt_buff* pkt) {
	/* Cast the pkt_buff to an ICMP type */
	icmp_packet* icmp_pkt = (icmp_packet*) pkt -> data;

	/* Set type and code. */
	icmp_pkt -> type = ECHO_REPLY;
	icmp_pkt -> code = 0x0;
	
	/* Set checksum to 0, then compute the real checksum */
	icmp_pkt -> checksum = 0x0;
	icmp_pkt -> checksum = htons(inet_checksum(icmp_pkt, pkt -> trans_length));
		
	/* Set relevant pkt_buff meta data, incluing network layer type and size */
	pkt -> t_type = TRANS_ICMP;
	pkt -> size = pkt -> trans_length;

	/* Send this IP stack. */
	//print_icmp_info(icmp_pkt);
	ip4_send(pkt);
}

/* Receive data from IP4 layer */
void icmp_recv(pkt_buff* pkt) {
	/* Cast to ICMP packet, then make decision */
	icmp_packet* icmp_pkt = (icmp_packet*) pkt -> data;
	print_icmp_info(icmp_pkt);
	kprintf("\n");

	/* Check the type, pass to the correct layer (also may need to cast payload) */
	uint8_t type = icmp_pkt -> type;
	handlers[type].handler(pkt);

	//print_icmp_info(packet);
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

	return (uint16_t) ~sum;
}
