#include <stdint.h>
#include <stddef.h>

#include <lib/string.h>
#include <lib/kprintf.h>

#include <mm/heap.h>

#include <net/arp.h>
#include <net/byteorder.h>
#include <net/eth.h>
#include <net/ip4.h>

#define ARP_CACHE_SIZE 10

/* Statically initialized ARP cache for now.
 * I want to look into creating a vector for this stuff, need to implement that and mem allocator.
 *
 * --- I was wrong, it turns out this should be a fixed size to prevent spoof attacks.
 * */
arp_cache_entry arp_cache[ARP_CACHE_SIZE];

static void print_mac(uint8_t* mac) {
	/* New lining left to the calling function */
	for(size_t pos = 0; pos < 6; pos++) {
		kprintf("%x:", mac[pos]);
	}
}

static void print_ip(uint8_t* ip) {
	/* New lining left to the calling function */
	for(size_t pos = 0; pos < 4; pos++) {
		kprintf("%d:", ip[pos]);
	}
}

static void print_arp_info(arp_packet* packet) {
	/* Print all relevant fields. Convert Byte Ordering.(See net/arp.h)*/
	uint16_t ht = ntohs(packet -> htype);
	uint16_t pt = ntohs(packet -> ptype);
	kprintf("HTYPE: %x, PTYPE: %x\n", ht, pt);

	uint8_t hl = packet -> hlen;
	uint8_t pl = packet -> plen;
	kprintf("HLEN: %x, PLEN: %x\n", hl, pl);

	uint16_t op = ntohs(packet -> oper);
	kprintf("OPER: %x\n", op);

	/* Sender MAC address. */
	kprintf("Sender MAC: ");
	print_mac(packet -> sha);

	/* Sender IPv4 (for now) address. */
	kprintf("\nSender IP: ");
	print_ip(packet -> spa);

	/* Target MAC address. */
	kprintf("\nTarget MAC: ");
	print_mac(packet -> tha);

	/* Target IPv4. */
	kprintf("\nTarget IP: ");
	print_ip(packet -> tpa);
	
	kprintf("\n");
}

static void print_arp_cache() {
	arp_cache_entry* cache = arp_cache;
	while(cache -> state) {
		/* Iterate through all good entries */
		kprintf("IP: ");
		print_ip(cache -> ip);
		kprintf("\n");

		kprintf("MAC: ");
		print_mac(cache -> mac);
		kprintf("\n");

		cache++;
	}

}

/* Update, or not, the contents of the ARP cache. */
static void arp_update_cache(arp_packet* packet) {
	arp_cache_entry* cache = arp_cache; 

	/* If this hits, we have a good entry.
	 * So just check IP and MAC match do ensure its not a dup */
	while(cache -> state) {
		int8_t ip_comp = memcmp(packet -> spa, cache -> ip, 4);  
		int8_t mac_comp = memcmp(packet -> sha, cache -> mac, 6);

		/* If they are both the same, memcmp returns 0 */
		if(ip_comp == 0 && mac_comp == 0) {
			return;
		}
		cache++;
	}

	/* Empty entry, or out of room. This needs to change */
	/* For now, just copy everything, and set to GOOD */
	memcpy(cache -> ip, packet -> spa, 4);
	memcpy(cache -> mac, packet -> sha, 6);
	cache -> state = GOOD;
}

/* Send a ARP request Packet */
static void arp_req() {
	
}

/* Send an ARP response packet */
static void arp_res(arp_packet* packet) {
	/* Create a new ARP packet, fill in the fields.
	 * Requires us to obtain our IP and MAC addresses */
	arp_packet* snd_pack = (arp_packet*) kmalloc(sizeof(arp_packet));

	/* Set hardware type, protocol type, hardware length, protocol length, op.
	 * Remember, convert to network endianess */
	snd_pack -> htype = htons(0x1);
	snd_pack -> ptype = htons(0x0800);
	snd_pack -> hlen = 0x6;
	snd_pack -> plen = 0x4;
	snd_pack -> oper = htons(0x2);

	/* Sender Hardware and IP Addresses are ours.
	 * --- Change get vs ret address calls. */
	memcpy(snd_pack -> sha, get_mac(), sizeof(snd_pack -> sha));
	memcpy(snd_pack -> spa, ret_ip(), sizeof(snd_pack -> spa));
	
	/* Target Hardware and IP addresses belong to requesting arp packet */
	memcpy(snd_pack -> tha, packet -> sha, sizeof(snd_pack -> tha));
	memcpy(snd_pack -> tpa, packet -> spa, sizeof(snd_pack -> tpa));

	/* Pass this down to ethernet layer.
	 * Eth layer needs to know destination MAC and size. */
 	eth_send(packet -> sha, snd_pack, sizeof(arp_packet), ARP_ETH_TYPE);	

	// print_arp_info(snd_pack);
}

/* Init first entry of ARP cache */
void init_arp_cache() {
	arp_cache -> state = EMPTY;
}

void arp_recv(pkt_buff* pkt) {
	/* Cast to ARP packet. Then , depending on the Operation field, we 
	 * know to either update ARP cache or create a response. */
	arp_packet* packet = (arp_packet*) pkt -> data;
	// print_arp_info(packet);
	
	/* Regardless of whether this is meant for us or not, we still update ARP cache. */
	arp_update_cache(packet);

	/* Then, verify IP address matches our IP.
	 * IPv4 address is statically defined in ip4.c */
	uint8_t* ip = ret_ip();
	if(!memcmp(ip, packet -> spa, sizeof(packet -> spa))) {
		return;
	}

	/* Check to see if we have to repsond. */
	if(ntohs(packet -> oper) & ARP_REQ) {
		/* Handle the ARP request. */
		kprintf("Handling Request\n");
		arp_res(packet);
	} 
}

/* Search the ARP cache for a MAC address (assume we have it for now) */

uint8_t* search_arp_cache(void* ip) {
	arp_cache_entry* cache_entry = arp_cache;

	/* TODO: Fix all other loops, ARP cache is fixed size */
	for(size_t entry = 0; entry < ARP_CACHE_SIZE; entry++) {
		if(!memcmp(cache_entry -> ip, ip, sizeof(cache_entry -> ip))) {
			return cache_entry -> mac;
		}
		cache_entry++;
	}

} 


















