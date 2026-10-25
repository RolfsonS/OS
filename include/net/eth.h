#ifndef ETH_H
#define ETH_H

/* Allow for net_dev_t struct to be used */
typedef struct net_dev_s net_dev_t;

typedef struct eth_frame_s{
	uint8_t dest[6];
	uint8_t src[6];
	uint16_t type;
	uint8_t payload[];
}__attribute__((packed)) eth_frame;

/* Register a network device to the networking stack.
 *
 * Param: net_dev_t* net_dev, Pointer to the network device to register.
 */
void register_net_dev(net_dev_t* net_dev);


/* Send data via an ethernet frame */
void eth_send(pkt_buff* pkt);


/* Print ethernet info: DEST, SRC, ETHERTYPE.
 *
 * Param: eth_frame* eth, Recv'ed Ethernet Frame to print info for.
 * */
void print_eth_info(eth_frame* eth);

/* Return a pointer to the MAC address */
uint8_t* get_mac();
#endif
