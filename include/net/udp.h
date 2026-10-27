#ifndef UDP_H
#define UDP_H

/* User Datagram Protocl */

typedef struct pkt_buff_s pkt_buff;

typedef struct upd_packet_s {
	uint16_t s_port;
	uint16_t d_port;
	uint16_t length;
	uint16_t chksm;
	uint8_t payload[];
}__attribute__((packed)) udp_packet;


void udp_recv(pkt_buff* pkt);

void udp_send(pkt_buff* pkt);






#endif
