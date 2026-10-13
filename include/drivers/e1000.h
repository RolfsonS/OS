#ifndef E1000_H
#define E1000_H

/* --------- Relevant Device Registers ----------- */
/* Deviec Control */
#define CTRL 	0x0

/* Device Status */
#define STATUS 	0x8

/* EEPROM/Flash Control/Data */
#define EECD 	0x10

/* EEPROM READ */
#define EERD 	0x14

/* Interrupt Cause Read */
#define ICR 	0xC0

/* Interrupt Mask Set/Read */
#define IMS 	0xD0

/* Receive Control */
#define RCTL 	0x100

/* Receive Descriptor Base Low */
#define RDBAL 	0x2800

/* Receive Descriptor Base High */
#define RDBAH 	0x2804

/* Receive Descriptor Length */
#define RDLEN 	0x2808

/* Receive Descriptor Head */
#define RDH 	0x2810

/* Receive Descriptor Tail */
#define RDT 	0x2818

/* Transmit Control */
#define TCTL 	0x400

/* Transmit Descriptor Base Low */
#define TDBAL 	0x3800

/* Transmit Descriptor Base High */
#define TDBAH 	0x3804

/* Transmit Descriptor Length */
#define TDLEN 	0x3808

/* Transmit Descriptor Head */
#define TDH 	0x3810 

/* Transmit Descriptor Tail */
#define TDT 	0x3818

/* Receive Address Low */
#define RAL 	0x5400

/* Receive Address High */
#define RAH 	0x5404

/* Flow Control Address Low */
#define FCAL 	0x28

/* Flow Control Address High */
#define FCAH 	0x2C

/* Flow Control Type */
#define FCT 	0x30

/* Flow Control Transmit Timer Value */
#define FCCTV 	0x170

/* Multicast Table Array */
#define MTA	0x5200

/* ---------- Relevant Device Register Bits -------- */
#define CTRL_ASDE 	0x10
#define CTRL_SLU  	0x20
#define CTRL_PHY_RST 	0x80000000
#define CTRL_ILOS 	0x80
#define CTRL_VME	0x40000000

#define RCTL_EN		0x2
#define RCTL_LPE	0x20
#define RCTL_LMB	0xC0
#define RCTL_RDMTS_L 	0x100
#define RCTL_RDMTS_H	0x200
#define RCTL_MO 	0x3000
#define RCTL_BAM 	0x8000
#define RCTL_BSIZE_L	0x10000
#define RCTL_BSIZE_H	0x20000
#define RCTL_BSEX 	0x2000000
#define RCTL_SECRC 	0x4000000

#define TCTL_EN 	0x2
#define TCTL_PSP	0x8
#define TCTL_CT 	0xF0
#define TCTL_COLD 	0x40000

#endif 






















