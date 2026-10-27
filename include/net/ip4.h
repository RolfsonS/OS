#ifndef IP4_H
#define IP4_H

/* Internet Procotol Version 4 */

#define IP4_VER  	0x4

typedef struct pkt_buff_s pkt_buff;

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
 }__attribute__((packed)) ip4_packet;

/* IPv4 Layer receiving a packet from the layer below. 
 *
 * Param: void* data, Data of the packet. Length is a field.
 */
void ip4_recv(pkt_buff* pkt);

void ip4_send(pkt_buff* pkt);

uint8_t* ret_ip(void);

#endif
