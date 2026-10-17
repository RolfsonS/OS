#ifndef ETH_H
#define ETH_H

/* Allow for net_dev_t struct to be used */
typedef struct net_dev_s net_dev_t;

/* Register a network device to the networking stack.
 *
 * Param: net_dev_t* net_dev, Pointer to the network device to register.
 */
void register_net_dev(net_dev_t* net_dev);

void eth_send(void);

void print_eth_info();


#endif
