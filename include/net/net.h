#ifndef NET_H
#define NET_H

/* Struct for representing a network device. 
 * Used as the interface between the lowest OSI model layer
 * and the device driver. The exact layer may change between WiFi, Eth, 
 * or Cell */
typedef struct net_dev_s {
	/* MAC address for device. */
	uint8_t MAC[6];
	
	/* Send packets to device driver */
	void (*send)(void);

	/* Receive packets from device driver */
	void (*recv)(void* data, size_t size);
} net_dev_t;

#endif
