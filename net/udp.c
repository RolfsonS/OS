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
#include <net/udp.h>

/* TODO: The next networking milestion after this is going to be either
 * TCP or user space sockets... but I am still a ways from that. 
 * For now, the kernel can pass UDP datagrams around */

static void print_udp_info(udp_packet* packet) {
	/* Remember Net -> Host endianess! */
	uint16_t s_port = ntohs(packet -> s_port);
	uint16_t d_port = ntohs(packet -> d_port);
	kprintf("S PORT: %d, D PORT %d \n", s_port, d_port);

	uint16_t len = ntohs(packet -> length);
	uint16_t chksm = ntohs(packet -> chksm);
	kprintf("LEN: %d, CHKSM: %d \n", len, chksm);

	/* Calculate the length of the payload (Header is 8 bytes)*/
	uint16_t p_len = len - 0x8;
	for(size_t pos = 0; pos < (p_len - 1); pos++) {
		kprintf("%c", *(packet -> payload + pos));
	}
	kprintf("\n");
}


/* Receive a UDP Datagram from the network layer. */
void udp_recv(pkt_buff* pkt) {
	/* Cast the Packet, and print for now */
	udp_packet* udp_pkt = (udp_packet*) pkt -> data;
	print_udp_info(udp_pkt);

	/* Echo the request */
	udp_send(pkt);
}

/* FIXME: This acts as an echo rn, fix in the future... */ 
/* Send the UDP Datagram to the Network Layer */
void udp_send(pkt_buff* pkt) {
	kprintf("Sending!\n");
	/* Cast data to a udp datagram */
	udp_packet* udp_pkt = (udp_packet*) pkt -> data;

	/* Set relevant packet metadata */
	pkt -> t_type = TRANS_UDP;
	pkt -> size = udp_pkt -> length;

	/* Swap the src and destination ports */
	uint16_t temp = udp_pkt -> s_port;
	udp_pkt -> s_port = udp_pkt -> d_port;
	udp_pkt -> d_port = temp;

	udp_pkt -> chksm = 0x0;

	ip4_send(pkt);
}



