#ifndef PIC_H
#define PIC_H

/* Configures the master/slave Programmable Interrupt Controller.
 * Expects 4 Initialzation Command Words (ICWs).
 * ICW1 starts intialization, ICW2 sets master and slave offsets.
 * ICW3 sets master and slave wiring.
 * ICW4 indicates 8086 mode.
 * Both PICs are then unmasked, allowing for nonblocking interrupts.
 */
void configure_pic(void);

/* Sends End of Interrupt command, acknowledging an interrupt. */
void pic_sendEOI(uint8_t);

#endif
