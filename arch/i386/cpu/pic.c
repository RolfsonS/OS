#include <stdint.h>
#include "pic.h"
#include "io.h"

/* Address ports on PIC we use */
#define PIC1 0x20  // Base address for master PIC
#define PIC2 0xA0  // Base address for slave PIC
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1+1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2+1)

/* When entering protected mode, we need to intialize the two PICs (Command port).
 * They then expect three other bytes: offset, master/slave wiring, additional info.
 * These #defines make things a little easier to understand.
 */
#define ICW1_ICW4 0x01 		// Indicates that ICW4 
#define ICW1_INIT 0x10 		// Required for Initialization

#define ICW4_8086 0x01 		// 8086 mode

/* New offsets we define.   
 * Interrupts 0-31 are CPU exceptions.
 */
#define OFFSET1 0x20
#define OFFSET2 0x28

/* For Master/Slave IRQ pins */
#define CASCADE_IRQ 2

/* End of Interrupt command */
#define PIC_EOI 0x20

void configure_pic(void) {
	/* Initialization */
	outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
	outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

	/* Master and slave vector offsets */
	outb(PIC1_DATA, OFFSET1);
	outb(PIC2_DATA, OFFSET2);

	/* Tell Master PIC there is a slave PIC at IRQ2 */
	outb(PIC1_DATA, 1 << CASCADE_IRQ);
	/* Tell Slave PIC its identity */
	outb(PIC2_DATA, CASCADE_IRQ);

	/* Use 8086 mode */
	outb(PIC1_DATA, ICW4_8086);
	outb(PIC2_DATA, ICW4_8086);

	/* Unmask both PICs */
	outb(PIC1_DATA, 0);
	outb(PIC2_DATA, 0);

}

void pic_sendEOI(uint8_t irq) {
	/* Send Interrupt Acknowledgments to PICs */
	if(irq >= 8) {
		outb(PIC2_COMMAND, PIC_EOI);
	}
	outb(PIC1_COMMAND, PIC_EOI);
}
