#include <inc/mmu.h>
#include <inc/string.h>
#include <inc/error.h>
#include <kern/e1000.h>
#include <kern/pmap.h>

// LAB 6: Your driver code here
volatile uint32_t *bar0;
struct e1000_tx_desc tx_desc_ring[TX_DESC_NUM] __attribute__((aligned(16)));
char tx_packet_buffers[TX_DESC_NUM][ETHERNET_PACKET_SIZE]; 

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
	}

	/* --- Transmit Initialization Begin --- */

	// Program the Transmit Descriptor Base Address (TDBAL/TDBAH) register(s) with
	// the address of the region.
	bar0[E1000_TDBAL] = PADDR(tx_desc_ring);
	bar0[E1000_TDBAH] = 0;

	// Set the Transmit Descriptor Length (TDLEN) register to the size (in bytes)
	// of the descriptor ring. This register must be 128-byte aligned.
	bar0[E1000_TDLEN] = sizeof(tx_desc_ring);

	// The Transmit Descriptor Head and Tail (TDH/TDT) registers are initialized
	// (by hardware) to 0b after a power-on or a software initiated Ethernet controller reset.
	// Software should write 0b to both these registers to ensure this.
	bar0[E1000_TDH] = 0;
	bar0[E1000_TDT] = 0;

	// Initialize the Transmit Control Register (TCTL)
	bar0[E1000_TCTL] |= E1000_TCTL_EN;
	bar0[E1000_TCTL] |= E1000_TCTL_PSP;
	bar0[E1000_TCTL] |= E1000_TCTL_CT & (0x10 << E1000_TCTL_CT_OFFSET);
	bar0[E1000_TCTL] |= E1000_TCTL_COLD & (0x40 << E1000_TCTL_COLD_OFFSET); // For the TCTL.COLD, you can assume full-duplex operation.

	// Program the Transmit IPG (TIPG) register to get the minimum legal Inter Packet Gap.
	// For TIPG, refer to the default values described in table 13-77 of section 13.4.34 for the IEEE 802.3 standard IPG.
	bar0[E1000_TIPG] = 0; // Clean bit
	bar0[E1000_TIPG] |= 10 << E1000_TIPG_IPGT_OFFSET;
	bar0[E1000_TIPG] |= 4 << E1000_TIPG_IPGR1_OFFSET;
	bar0[E1000_TIPG] |= 6 << E1000_TIPG_IPGR2_OFFSET;

	/* --- Transmit Initialization End --- */

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


	return 0;
}

int e1000_transmit(char *data, uint32_t len) {
	// Note that TDT is an index into the transmit descriptor array, not a byte offset
	uint32_t tdt;

	if (data == NULL || len > ETHERNET_PACKET_SIZE)
		return -E_INVAL;

	// Checking if the next descriptor is free
	tdt = bar0[E1000_TDT];
	if (!(tx_desc_ring[tdt].upper.data & E1000_TXD_STAT_DD))
		return -E_TRANSMIT_QUEUE_FULL;

	// Copy the packet data into the next descriptor
	memset(tx_packet_buffers[tdt], 0, sizeof(tx_packet_buffers[tdt]));
	memmove(tx_packet_buffers[tdt], data, len);
	
	tx_desc_ring[tdt].lower.flags.length = len;
	tx_desc_ring[tdt].lower.data |= E1000_TXD_CMD_RS;
	tx_desc_ring[tdt].lower.data |= E1000_TXD_CMD_EOP;
	tx_desc_ring[tdt].upper.data &= ~E1000_TXD_STAT_DD;

	// Update TDT
	bar0[E1000_TDT] = (tdt + 1) % TX_DESC_NUM;

	return 0;
}
