#ifndef NET_H
#define NET_H

/* Background -- there were lots of problems with passing parameters up and down the 
 * stack. For example, IP -> ICMP, the ICMP packet has no way of remembering the 
 * senders IP address, so sending back down requires IP to actually send this info up.
 * This becomes extremely tedious and wasteful.
 *
 * Instead pkt_buff stores information regarding MAC, IP, and Data and passese this around instead.
 * This requires upgrading... want to use a single buffer in the future (Linux skb style).
 */
typedef struct pkt_buff_s {
	uint8_t MAC[6];
	uint8_t IP[4];
	void* data;
} pkt_buff;

/* Struct for representing a network device. 
 * Used as the interface between the lowest OSI model layer
 * and the device driver. The exact layer may change between WiFi, Eth, 
 * or Cell */
typedef struct net_dev_s {
	/* MAC address for device. */
	uint8_t MAC[6];
	
	/* Send packets to device driver */
	void (*send)(void* data, size_t size);

	/* Receive packets from device driver */
	void (*recv)(pkt_buff* pkt);
} net_dev_t;

#endif
