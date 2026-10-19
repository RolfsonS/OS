/* Address Resolution Protocol */

#define ARP_REQ 	0x1
#define ARP_REP 	0x2

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

/* ARP layer receives a packet from lower layer.
 *
 * Param: void* data, ARP Packet 
 */
void arp_recv(void* data);
