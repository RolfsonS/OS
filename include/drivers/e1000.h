#ifndef E1000_H
#define E1000_H

/* Relevant Device Registers */

/* Deviec Control */
#define CTRL 0x0

/* Device Status */
#define STATUS 0x8

/* EEPROM/Flash Control/Data */
#define EECD 0x10

/* EEPROM READ */
#define EERD 0x14

/* Interrupt Cause Read */
#define ICR 0xC0

/* Interrupt Mask Set/Read */
#define IMS 0xD0

/* Receive Control */
#define RCTL 0x100

/* Receive Descriptor Base Low */
#define RDBAL 0x2800

/* Receive Descriptor Base High */
#define RDBAH 0x2804

/* Receive Descriptor Length */
#define RDLEN 0x2808

/* Receive Descriptor Head */
#define RDH 0x2810

/* Receive Descriptor Tail */
#define RDT 0x2818

/* Transmit Control */
#define TCTL 0x400

/* Transmit Descriptor Base Low */
#define TDBAL 0x3800

/* Transmit Descriptor Base High */
#define TDBAH 0x3804

/* Transmit Descriptor Length */
#define TDLEN 0x3808

/* Transmit Descriptor Head */
#define TDH 0x3810 

/* Transmit Descriptor Tail */
#define TDT 0x3818

#endif 
