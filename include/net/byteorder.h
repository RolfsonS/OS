#ifndef BYTEORDER_H
#define BYTEORDER_H

/* Host to Network Short (Little to Big Endian) */
static inline uint16_t htons(uint16_t data) {
	return (uint16_t) (data << 8) | (data >> 8);
}

/* Host to Network Long (Little to Big Endian) */
static inline uint32_t htonl(uint32_t data) {
	return (uint32_t) (data << 24) | 
			  ((data << 8) & 0x00FF0000) |
			  (data >> 24) | 
			  ((data >> 8) & 0x0000FF00);
}

/* Network to Host Short (Big to Little Endian) */
static inline uint16_t ntohs(uint16_t data) {
	return (uint16_t) (data << 8) | (data >> 8);
}

/* Network to Host Long (Bif to Little Endian) */
static inline uint32_t ntohl(uint32_t data) {
	return (uint32_t) (data << 24) | 
			  ((data << 8) & 0x00FF0000) |
			  (data >> 24) | 
			  ((data >> 8) & 0x0000FF00);
}

#endif
