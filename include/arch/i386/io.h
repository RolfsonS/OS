#ifndef IO_H
#define IO_H

/* Write a byte to a port. Assembly.
 *
 * Param: uint16_t port, the output port.
 * Param: uint8_t val, the byte value to right to the port.
 */
void outb(uint16_t port, uint8_t val);

/* Write 2 bytes to a port. Assembly.
 *
 * Param: uint16_t port, the output port.
 * Param: uint16_t val, the word value to right to the port.
 */
void outw(uint16_t port, uint16_t val);

/* Write 4 bytes to a port. Assembly.
 *
 * Param: uint16_t port, the output port.
 * Param: uint32_t val, the long value to right to the port.
 */
void outl(uint16_t, uint32_t val);

/* Return a byte from a port.
 * 
 * Param: uint16_t port, the input port to read from.
 * 
 * Return: uint8_t, the byte from the input port returned.
 */
uint8_t inb(uint16_t port);

/* Return 2 bytes from a port.
 * 
 * Param: uint16_t port, the input port to read from.
 * 
 * Return: uint16_t, the byte from the input port returned.
 */
uint16_t inw(uint16_t port);

/* Return 4 bytes from a port.
 * 
 * Param: uint16_t port, the input port to read from.
 * 
 * Return: uint8_t, the byte from the input port returned.
 */
uint32_t inl(uint16_t port);


#endif
