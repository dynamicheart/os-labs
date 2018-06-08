#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <kern/pci.h>

#define TX_DESC_NUM 64
#define ETHERNET_PACKET_SIZE 1518

/* PCI Device IDs */
#define E1000_VEN_ID_82540EM 0x8086
#define E1000_DEV_ID_82540EM 0x100E

/* Register Set. divided by 4 for use as uint32_t[] indices
 *
 * Registers are defined to be 32 bits and  should be accessed as 32 bit values.
 * These registers are physically located on the NIC, but are mapped into the
 * host memory address space.
 *
 * RW - register is both readable and writable
 * RO - register is read only
 * WO - register is write only
 * R/clr - register is read only and is cleared when read
 * A - register array
 */
#define E1000_STATUS   (0x00008/4)  /* Device Status - RO */
#define E1000_TCTL     (0x00400/4)  /* TX Control - RW */
#define E1000_TIPG     (0x00410/4)  /* TX Inter-packet gap -RW */
#define E1000_TDBAL    (0x03800/4)  /* TX Descriptor Base Address Low - RW */
#define E1000_TDBAH    (0x03804/4)  /* TX Descriptor Base Address High - RW */
#define E1000_TDLEN    (0x03808/4)  /* TX Descriptor Length - RW */
#define E1000_TDH      (0x03810/4)  /* TX Descriptor Head - RW */
#define E1000_TDT      (0x03818/4)  /* TX Descripotr Tail - RW */

/* Transmit Control */
#define E1000_TCTL_EN            0x00000002    /* enable tx */
#define E1000_TCTL_PSP           0x00000008    /* pad short packets */
#define E1000_TCTL_CT            0x00000ff0    /* collision threshold */
#define E1000_TCTL_COLD          0x003ff000    /* collision distance */
/* Transmit Control Offset */
#define E1000_TCTL_CT_OFFSET     4             /* offset of collision threshold */
#define E1000_TCTL_COLD_OFFSET   12            /* offset of collision distance */

/* Transmit Descriptor bit definitions */
#define E1000_TXD_CMD_EOP    0x01000000 /* End of Packet */
#define E1000_TXD_CMD_RS     0x08000000 /* Report Status */
#define E1000_TXD_STAT_DD    0x00000001 /* Descriptor Done */

/* Inter Packet Gap Control Offset */
#define E1000_TIPG_IPGT_OFFSET   0    /* Offset of IPG Transmit Time */
#define E1000_TIPG_IPGR1_OFFSET  10   /* Offset of IPG Receive Time 1 */
#define E1000_TIPG_IPGR2_OFFSET  20   /* Offset of IPG Receive Time 2 */

/* Transmit Descriptor */
struct e1000_tx_desc {
	uint64_t buffer_addr;       /* Address of the descriptor's data buffer */
	union {
		uint32_t data;
		struct {
			uint16_t length;    /* Data buffer length */
			uint8_t cso;        /* Checksum offset */
			uint8_t cmd;        /* Descriptor control */
		} flags;
	} lower;
	union {
		uint32_t data;
		struct {
			uint8_t status;     /* Descriptor status */
			uint8_t css;        /* Checksum start */
			uint16_t special;
		} fields;
	} upper;
};

/* Receive Descriptor */
struct e1000_rx_desc {
	uint64_t buffer_addr; /* Address of the descriptor's data buffer */
	uint16_t length;     /* Length of data DMAed into data buffer */
	uint16_t csum;       /* Packet checksum */
	uint8_t status;      /* Descriptor status */
	uint8_t errors;      /* Descriptor Errors */
	uint16_t special;
};

int e1000_82540em_attach(struct pci_func *pcif);
int e1000_transmit(char *data, uint32_t len);

#endif	// JOS_KERN_E1000_H
