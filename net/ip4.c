#include <stdint.h>
#include <stddef.h>

#include <lib/kprintf.h>

#include <net/ip4.h>
#include <net/eth.h>
#include <net/byteorder.h>
#include <net/icmp.h>

/* Macros for certain IP4 packet fields */
#define GET_VERSION(n)	((n) >> 4u)
#define GET_IHL(n) 	((n) & 0x0F)
#define GET_DSCP(n)	((n) >> 2u)
#define GET_ENC(n)	((n) & 0x3)
#define	GET_FLAGS(n)	((n) >> 13)
#define GET_FRAG(n)	((n) & 0x1FFF)


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

/* IP packet received from layer below. */
void ip4_recv(void* data) {
	/* Cast to IP packet, assign to correct protocol. 
	 * TODO: Drop if not matching IP or correct checksum. */
	ip4_packet* packet = (ip4_packet*) data;
	uint8_t protocol = packet -> proto;

	if(protocol == IP4_ICMP) {
		icmp_recv(packet -> payload);
	} 
	print_ip4_info(packet);
}

uint8_t* ret_ip(void) {
	return ip4_addr;
}
