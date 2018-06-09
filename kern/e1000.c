#include <inc/mmu.h>
#include <inc/string.h>
#include <inc/error.h>

#include <kern/e1000.h>
#include <kern/pmap.h>
#include <kern/env.h>

// LAB 6: Your driver code here
volatile uint32_t *bar0;

struct e1000_rx_desc rx_desc_ring[RX_DESC_NUM]__attribute__((aligned(16)));
char rx_packet_buffers[RX_DESC_NUM][RX_PACKET_SIZE];

struct e1000_tx_desc tx_desc_ring[TX_DESC_NUM] __attribute__((aligned(16)));
char tx_packet_buffers[TX_DESC_NUM][TX_PACKET_SIZE];

static int mmio_map(void *vaddr, uint32_t paddr, uint32_t size, uint32_t perm) {
	void *last;
	pte_t *pte;

	vaddr = ROUNDDOWN(vaddr, PGSIZE);
	last = ROUNDDOWN(vaddr + size - 1, PGSIZE);
	for(;;) {
		if((pte = pgdir_walk(kern_pgdir, vaddr, 1)) == NULL)
			return -E_NO_MEM;
		if((*pte & PTE_P))
			return -E_INVAL;
		*pte = paddr | perm;

		if(vaddr == last)
			break;

		vaddr += PGSIZE;
		paddr += PGSIZE;
	}

	return 0;
}

static int e1000_setup_tx_resources() {
	memset(tx_desc_ring, 0, sizeof(tx_desc_ring));
	memset(tx_packet_buffers, 0, sizeof(tx_packet_buffers));
	for (int i = 0; i < TX_DESC_NUM; i++) {
		tx_desc_ring[i].buffer_addr = PADDR(tx_packet_buffers[i]);
		tx_desc_ring[i].upper.data |= E1000_TXD_STAT_DD;
		tx_desc_ring[i].lower.data |= E1000_TXD_CMD_RS | E1000_TXD_CMD_EOP;
	}

	/* --- Transmit Initialization Begin --- */
	bar0[E1000_TDBAL] = PADDR(tx_desc_ring);
	bar0[E1000_TDBAH] = 0;

	bar0[E1000_TDLEN] = sizeof(tx_desc_ring);

	bar0[E1000_TDH] = 0;
	bar0[E1000_TDT] = 0;

	bar0[E1000_TCTL] |= E1000_TCTL_EN;
	bar0[E1000_TCTL] |= E1000_TCTL_PSP;
	bar0[E1000_TCTL] |= E1000_TCTL_CT & (0x10 << E1000_TCTL_CT_SHIFT);
	bar0[E1000_TCTL] |= E1000_TCTL_COLD & (0x40 << E1000_TCTL_COLD_SHIFT); // For the TCTL.COLD, you can assume full-duplex operation.

	// For TIPG, refer to the default values described in table 13-77 of section 13.4.34 for the IEEE 802.3 standard IPG.
	bar0[E1000_TIPG] = 0; // Clean bit
	bar0[E1000_TIPG] |= 10 << E1000_TIPG_IPGT_SHIFT;
	bar0[E1000_TIPG] |= 4 << E1000_TIPG_IPGT1_SHIFT;
	bar0[E1000_TIPG] |= 6 << E1000_TIPG_IPGT2_SHIFT;
	/* --- Transmit Initialization End --- */

	return 0;
}

static int e1000_setup_rx_resources() {
	memset(rx_desc_ring, 0, sizeof(rx_desc_ring));
	memset(rx_packet_buffers, 0, sizeof(rx_packet_buffers));
	for (int i = 0; i < RX_DESC_NUM; i++) {
		rx_desc_ring[i].buffer_addr = PADDR(rx_packet_buffers[i]);
		rx_desc_ring[i].status &= ~E1000_RXD_STAT_DD;
	}

	/* --- Receive Initialization Begin --- */
	// Hard-code QEMU's default MAC address of 52:54:00:12:34:56
	bar0[E1000_RAL] = 0x12005452;
	bar0[E1000_RAH] = 0x00005634 | E1000_RAH_AV; // Don't forget to set the "Address Valid" bit in RAH.

	// You don't have to support "long packets" or multicast.
	// For now, don't configure the card to use interrupts.

	bar0[E1000_RDBAL] = PADDR(rx_desc_ring);
	bar0[E1000_RDBAH] = 0;

	bar0[E1000_RDLEN] = sizeof(rx_desc_ring);

	bar0[E1000_RDH] = 0;
	bar0[E1000_RDT] = RX_DESC_NUM - 1;

	bar0[E1000_RCTL] &= ~E1000_RCTL_LPE;

	bar0[E1000_RCTL] &= ~E1000_RCTL_LBM_MASK;
	bar0[E1000_RCTL] |= E1000_RCTL_LBM_NO;

	bar0[E1000_RCTL] &= ~E1000_RCTL_MPE;

	bar0[E1000_RCTL] &= ~E1000_RCTL_RDMTS_MASK;
	bar0[E1000_RCTL] |= E1000_RCTL_RDMTS_HALF;

	bar0[E1000_RCTL] |= E1000_RCTL_BAM;

	bar0[E1000_RCTL] &= ~E1000_RCTL_BSEX;
	bar0[E1000_RCTL] &= ~E1000_RCTL_SZ_MASK;
	bar0[E1000_RCTL] |= E1000_RCTL_SZ_2048;

	bar0[E1000_RCTL] |= E1000_RCTL_SECRC;

	// It is best to leave the Ethernet controller receive logic disabled (RCTL.EN = 0b)
	// until after the receive descriptor ring has been initialized and software is ready
	// to process received packets.
	bar0[E1000_RCTL] |= E1000_RCTL_EN;

	/* --- Receive Initialization End --- */
	return 0;
}

int e1000_82540em_attach(struct pci_func *pcif) {
	int res;

	pci_func_enable(pcif);

	// Create a virtual memory mapping from KSTACKTOP to E1000's BAR 0
	// bar0's size is less than 4MB
	if ((res = mmio_map((void *)KSTACKTOP, pcif->reg_base[0], 
		pcif->reg_size[0], PTE_P | PTE_W | PTE_PCD | PTE_PWT)) < 0)
		panic("e1000_82540em_attach: mmio map failed");
	bar0 = (uint32_t *)KSTACKTOP;

	// To test your mapping, try printing out the device status register (section 13.4.2).
	// This is a 4 byte register that starts at byte 8 of the register space. You should 
	// get 0x80080783, which indicates a full duplex link is up at 1000 MB/s, among other 
	// things.
	assert(bar0[E1000_STATUS] == 0x80080783);

	if ((res = e1000_setup_tx_resources()) < 0)
		panic("e1000_82540em_attach: setup tx failed");

	if ((res = e1000_setup_rx_resources()) < 0)
		panic("e1000_82540em_attach: setup rx failed");

	return 0;
}

int e1000_transmit(void *data, uint32_t len) {
	// Note that TDT is an index into the transmit descriptor array, not a byte offset
	uint32_t tdt;

	// Checking if the next descriptor is free
	tdt = bar0[E1000_TDT];
	if (!(tx_desc_ring[tdt].upper.data & E1000_TXD_STAT_DD))
		return -E_TRANSMIT_QUEUE_FULL;

	// Copy the packet data into the next descriptor
	memset(tx_packet_buffers[tdt], 0, sizeof(tx_packet_buffers[tdt]));
	memmove(tx_packet_buffers[tdt], data, len);

	// RS bit and EOP bit are set in initialization
	tx_desc_ring[tdt].lower.flags.length = len;
	tx_desc_ring[tdt].upper.data &= ~E1000_TXD_STAT_DD;

	// Update TDT
	bar0[E1000_TDT] = (tdt + 1) % TX_DESC_NUM;

	return 0;
}

int e1000_receive(void *data_store) {
	int res;
	uint32_t new_rdt;
	uint16_t len;

	new_rdt = (bar0[E1000_RDT] + 1) % RX_DESC_NUM;
	if (!(rx_desc_ring[new_rdt].status & E1000_RXD_STAT_DD))
		return -E_NO_RECV_PACKET;

	assert(rx_desc_ring[new_rdt].status & E1000_RXD_STAT_EOP);

	len = rx_desc_ring[new_rdt].length;
	assert(len > 0 && len <= RX_PACKET_SIZE);

	memmove(data_store, rx_packet_buffers[new_rdt], len);

	rx_desc_ring[new_rdt].status &= ~E1000_RXD_STAT_DD;
	rx_desc_ring[new_rdt].status &= ~E1000_RXD_STAT_EOP;

	bar0[E1000_RDT] = new_rdt;

	return len;
}
