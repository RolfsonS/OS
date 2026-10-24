/* Address Resolution Protocol */

#define ARP_REQ 	0x1
#define ARP_REP 	0x2

#define EMPTY 		0x0
#define GOOD 		0x1

/* ARP Packet Structure */
typedef struct arp_packet_t {
	uint16_t htype; 	/* Link protocol */
	uint16_t ptype;		/* Network protocol */
	uint8_t hlen;		/* Length of hardware address */
	uint8_t plen;		/* Length of network protocol address */
	uint16_t oper;		/* Operation - 1 for request, 2 for reply */
	uint8_t sha[6];		/* Sender hardware address */
	uint8_t spa[4];		/* Network address of the sender */
	uint8_t tha[6];		/* Target hardware address */
	uint8_t tpa[4];		/* Target network address */
} __attribute__((packed)) arp_packet;

typedef struct arp_cache_entry_s {
	uint8_t ip[4];
	uint8_t mac[6];
	uint8_t state;
} arp_cache_entry;

/* ARP layer receives a packet from lower layer.
 *
 * Param: void* data, ARP Packet 
 */
void arp_recv(pkt_buff* pkt);

void init_arp_cache();

uint8_t* search_arp_cache(void* data);
