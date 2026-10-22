/* Internet Procotol Version 4 */

#define IP4_ICMP	0x1
#define IP4_TCP 	0x6
#define IP4_UDP 	0x11

typedef struct ip4_packet_S {
	uint8_t version_ihl;
	uint8_t dcsp_ecn;
	uint16_t len;
	uint16_t id;
	uint16_t flag_frag;
	uint8_t ttl;
	uint8_t proto;
	uint16_t h_chksm;
	uint8_t source_addr[4];
	uint8_t dest_addr[4];
	uint8_t payload[];
} __attribute__((packet)) ip4_packet;

/* IPv4 Layer receiving a packet from the layer below. 
 *
 * Param: void* data, Data of the packet. Length is a field.
 */
void ip4_recv(void* data);

void ip4_send(void* dest_ip, void* packet, size_t size, uint16_t type);

uint8_t* ret_ip(void);
