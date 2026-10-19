#include <stdint.h>
#include <stddef.h>

#include <lib/string.h>
#include <lib/kprintf.h>

#include <net/arp.h>
#include <net/byteorder.h>
#include <net/eth.h>
#include <net/ip4.h>

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
	for(size_t mac = 0; mac < 6; mac++) {
		kprintf("%x:", packet -> sha[mac]);
	}

	/* Sender IPv4 (for now) address. */
	kprintf("\nSender IP: ");
	for(size_t ip = 0; ip < 4; ip++) {
		kprintf("%d.", packet -> spa[ip]);
	}

	/* Target MAC address. */
	kprintf("\nTarget MAC: ");
	for(size_t mac = 0; mac < 6; mac++) {
		kprintf("%x:", packet -> tha[mac]);
	}

	/* Target IPv4. */
	kprintf("\nTarget IP: ");
	for(size_t ip = 0; ip < 4; ip++) {
		kprintf("%x.", packet -> tpa[ip]);
	}
	kprintf("\n");
}

static void arp_update_cache(arp_packet* packet) {
	
}

static void arp_req() {

}

void arp_recv(void* data) {
	/* Cast to ARP packet. Then , depending on the Operation field, we 
	 * know to either update ARP cache or create a response. */
	arp_packet* packet = (arp_packet*) data;
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
		arp_req();
	} 
}


