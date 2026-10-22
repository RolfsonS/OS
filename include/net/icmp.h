#ifndef ICMP_H
#define ICMP_H

#define ECHO_REPLY 	0x0
#define ECHO_REQUEST 	0x8

typedef struct icmp_packet_s {
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	/* Rest may be cast to anything else, depending on prev fields */
	uint8_t rest[];
} __attribute__((packed)) icmp_packet;

/* Receive packet from IP layer. */
void icmp_recv(void* data, void* ip_addr);

/* Compute inet cehcksum */
uint16_t inet_checksum(void* data, size_t size);
#endif
