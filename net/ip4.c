#include <stdint.h>
#include <stddef.h>

#include <lib/kprintf.h>
#include <lib/string.h>

#include <mm/heap.h>

#include <net/ip4.h>
#include <net/eth.h>
#include <net/byteorder.h>
#include <net/icmp.h>
#include <net/net.h>

/* Macros for certain IP4 packet fields */
#define GET_VERSION(n)	((n) >> 4u)
#define GET_IHL(n) 	((n) & 0x0F)
#define GET_DSCP(n)	((n) >> 2u)
#define GET_ENC(n)	((n) & 0x3)
#define	GET_FLAGS(n)	((n) >> 13u)
#define GET_FRAG(n)	((n) & 0x1FFF)

#define SET_VER_IHL(n, m) 	(((n) << 4u)| ((m) & 0xF))
#define SET_DSCP_ECN(n, m)	(((n) << 2u) | ((m) & 0x3))
#define SET_FLAGS_FRAGS(n, m)	(((n) << 13u) | ((m) & 0x1FFF)) 

/* There 256 possible IP protocols */
#define IP_PROTOS 	0xFF

/* Array based handling based on the IP packets type. */
typedef struct ip_proto_handler_t {
	void (*handler) (pkt_buff* pkt);
} ip_proto_handler;

/* Array for all the handlers */
static const ip_proto_handler handlers[IP_PROTOS] = {
	[TRANS_ICMP] = {
		.handler = icmp_recv,
	},
};

/* Statically defined, global IP address */
static const uint8_t ip4_addr[4] = {192, 168, 100, 2}; 

/* Print an IPv4 address */
static void print_ip(uint8_t* ip) {
	/* New lining left to the calling function */
	for(size_t pos = 0; pos < 4; pos++) {
		kprintf("%d:", ip[pos]);
	}
}

/* Print the header info of an IPv4 Packet. */
static void print_ip4_info(ip4_packet* packet) {
	uint8_t ver_ihl = packet -> version_ihl;
	kprintf("Version: %x, IHL: %x, ", GET_VERSION(ver_ihl), GET_IHL(ver_ihl));

	uint8_t dscp_ecn = packet -> dcsp_ecn;
	kprintf("DSCP: %x, ECN: %x\n", GET_DSCP(dscp_ecn), GET_ENC(dscp_ecn));
	
	kprintf("LEN: %d, ID: %x, ", ntohs(packet -> len), ntohs(packet -> id));

	uint16_t fl_fr = ntohs(packet -> flag_frag);
	kprintf("FLAGS: %x, FRAG: %x \n", GET_FLAGS(fl_fr), GET_FRAG(fl_fr));
	
	kprintf("TTL: %x, PROTO: %x\n", packet -> ttl, packet -> proto);

	kprintf("CHEKSUM: %x \n", ntohs(packet -> h_chksm));

	kprintf("SOURCE IP: ");
	print_ip(packet -> source_addr);
	kprintf(" DEST IP: ");
	print_ip(packet -> dest_addr);
	kprintf("\n");
} 

/* Send an IP4 packet to Ethernet layer */
void ip4_send(pkt_buff* pkt) {
	/* Move head... and cast to an ip4 packet. */
	pkt -> data -= sizeof(ip4_packet);
	ip4_packet* ip4_pkt = (ip4_packet*) pkt -> data;

	/* Set packet Meta data. */
	pkt -> n_type = NET_IP4; 
	pkt -> size += sizeof(ip4_packet);

	/* Fill in necessary fields. */
	ip4_pkt -> version_ihl = SET_VER_IHL(IP4_VER, 0x5);
	ip4_pkt -> dcsp_ecn = SET_DSCP_ECN(0x0, 0x0);
	ip4_pkt -> len = htons(pkt -> trans_length + sizeof(ip4_packet));
	ip4_pkt -> id = htons(0x0);
	ip4_pkt -> flag_frag = htons(SET_FLAGS_FRAGS(0x0, 0x0));
	ip4_pkt -> ttl = 0x50;
	
	/* For the protocol, this was passed down via meta data. */
	ip4_pkt -> proto = pkt -> t_type;

	/* Let checksum = 0x0, then recalculate once every other field is complete */
	ip4_pkt -> h_chksm = 0x0;

	/* Set the host and sender address via memcpy */
	memcpy(ip4_pkt -> source_addr, ret_ip(), sizeof(pkt -> IP));
	memcpy(ip4_pkt -> dest_addr, pkt -> IP, sizeof(pkt -> IP));

	/* Calculate the checksum. */
	uint16_t chksm = htons(inet_checksum(ip4_pkt, sizeof(ip4_packet)));
	ip4_pkt -> h_chksm = chksm;

	/* Send to ethernet layer. */
	eth_send(pkt);
}

/* IP packet received from layer below. */
void ip4_recv(pkt_buff* pkt) {
	/* Cast to IP packet, assign to correct protocol. 
	 * TODO: Drop if not matching IP or correct checksum (Or options) */
	ip4_packet* ip4_pkt = (ip4_packet*) pkt -> data;
	uint8_t protocol = ip4_pkt -> proto;

	/* Set the pkt_buff's IP address and transport length. */
	memcpy(pkt -> IP, ip4_pkt -> source_addr, sizeof(pkt -> IP));

	/* The IHL field indicates amount of 32 bit entries, so mutliply by 4 for bytes */
	size_t header_len = GET_IHL(ip4_pkt -> version_ihl) << 2;
	size_t total_len  = ntohs(ip4_pkt -> len);
	pkt -> trans_length = total_len - header_len;

	/* Increment... FIX! */
	pkt -> data += header_len;

	/* Call the relevant L4 Protocol. */
	handlers[protocol].handler(pkt);

	//print_ip4_info((ip4_packet*) pkt -> data);
}

uint8_t* ret_ip(void) {
	return ip4_addr;
}
