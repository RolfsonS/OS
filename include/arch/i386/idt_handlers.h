#ifndef IDT_HANDLERS_H
#define IDT_HANDLERS_H

/* This Header file contains all the Interrupt handlers */

/* General IDT handler, do nothing */
void general_idt_handler(void);

/* Interrupt Handler 0 - Timer Interrupt */
void irq_0_handler(void);

/* Interrupt Handler 1 - Keyboard Interrupt */
void irq_1_handler(void);

/* More to come... */ 
#endif
