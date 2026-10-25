#ifndef NET_H
#define NET_H

/* OSI L3 types, multiplexed via L2 */
typedef enum network_type_t {
	NET_IP4 = 0x0800,
	/* ARP actually exist more as 2.5 layer... */
	NET_ARP = 0x0806
} network_type;

/* OSI L4 types, multiplexed via L3 */
typedef enum transport_type_t {
	TRANS_ICMP = 0x1,
	TRANS_TCP = 0x6,
	TRANS_UDP = 0x11
} transport_type;


/* Background -- there were lots of problems with passing parameters up and down the 
 * stack. For example, IP -> ICMP, the ICMP packet has no way of remembering the 
 * senders IP address, so sending back down requires IP to actually send this info up.
 * This becomes extremely tedious and wasteful.
 *
 * Instead pkt_buff stores information regarding MAC, IP, and Data and passese this around instead.
 * This requires upgrading... want to use a single buffer in the future (Linux skb style).
 */
typedef struct pkt_buff_s {
	/* MAC and IP addresses, added coming up stack */
	uint8_t MAC[6];
	uint8_t IP[4];

	/* Pointer to the actual network packets */
	uint8_t* data;

	/* Current size of data */
	size_t size;

	/* Define layer protocol.
	 * For example, ARP -> ETH defines an ARP Ethernet type.
	 * Or, UDP/TCP -> IP defines their types */
	network_type n_type;
	transport_type t_type;

	/* Transport layer protocols rely on the network layer 
	 * for determining the size of the packet, provided by 
	 * this field. */
	size_t trans_length;
} pkt_buff;

/* Struct for representing a network device. 
 * Used as the interface between the lowest OSI model layer
 * and the device driver. The exact layer may change between WiFi, Eth, 
 * or Cell */
typedef struct net_dev_s {
	/* MAC address for device. */
	uint8_t MAC[6];
	
	/* Send packets to device driver */
	void (*send)(pkt_buff* pkt);

	/* Receive packets from device driver */
	void (*recv)(pkt_buff* pkt);
} net_dev_t;

/* Pull the data forward (towards the start) */
static inline void pkt_buff_pull(pkt_buff* pkt, size_t size) {
	pkt -> data -= size;
}

/* Push the data back (away from start) */
static inline void pkt_buff_push(pkt_buff* pkt, size_t size) {
	pkt -> data += size;
}

#endif





