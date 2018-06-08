
#include "fs.h"

#ifdef USE_EVICT_POLICY
static void* bc_list[NBLOCKCACHE];
static uint32_t next_free_bc = 0;
static uint32_t clock_arm = 0;

// Use clock page-removal algorithm
static void clock_bc_pgfault(void *addr, uint32_t blockno) {
	int r;
	void* candidate_addr;

	static_assert(NBLOCKCACHE > 0);
	assert((uint32_t)addr % PGSIZE == 0);
	if (next_free_bc < NBLOCKCACHE) {
		cprintf("No eviction!! Map block %u to 0x%08x\n", blockno, (uint32_t)addr);
		if ((r = sys_page_alloc(0, addr, PTE_P | PTE_U | PTE_W)) < 0)
			panic("sys_page_alloc: %e", r);
		if ((r = ide_read(blockno * BLKSECTS, addr, BLKSECTS)) < 0)
			panic("ide_read: %e", r);
		// Clear dirty bit
		if ((r = sys_page_map(0, addr, 0, addr, PTE_SYSCALL)) < 0)
			panic("sys_page_map: %e", r);

		bc_list[next_free_bc++] = addr;
	} else {
		// 1. Find the eviction candidate
		while(1) {
			candidate_addr = bc_list[clock_arm];
			if (vpt[PGNUM(candidate_addr)] & PTE_A) {
				// 2. Flush the candidate because both the dirty bit and access bit are cleared
				if (vpt[PGNUM(candidate_addr)] & PTE_D)
					flush_block(candidate_addr);
				// 3. Clear access bit and dirty bit
				if ((r = sys_page_map(0, candidate_addr, 0, candidate_addr, PTE_SYSCALL)) < 0)
					panic("sys_page_map: %e", r);
				clock_arm = (clock_arm + 1) % NBLOCKCACHE;
			} else
				break;
		}

		cprintf("Eviction!! Evict block %u. Clock arm %u\n", blockno, clock_arm);
		cprintf("Evict 0x%08x with 0x%08x\n",(uint32_t)candidate_addr, (uint32_t)addr);

		// 2. Flush the candidate
		if (vpt[PGNUM(candidate_addr)] & PTE_D)
			flush_block(candidate_addr);

		// 3. Map new page
		if ((r = sys_page_map(0, candidate_addr, 0, addr, PTE_SYSCALL)) < 0)
			panic("sys_page_map: %e", r);

		// 4. Unmap the old page
		if ((r = sys_page_unmap(0, candidate_addr)) < 0)
			panic("sys_page_unmap: %e", r);

		// 5. Read data from disk to memory
		if ((r = ide_read(blockno * BLKSECTS, addr, BLKSECTS)) < 0)
			panic("ide_read: %e", r);

		// 6. Clear dirty bit
		if ((r = sys_page_map(0, addr, 0, addr, PTE_SYSCALL)) < 0)
			panic("sys_page_map: %e", r);

		// 7. Update bc list
		bc_list[clock_arm] = addr;
	}
}
#endif


// Return the virtual address of this disk block.
void*
diskaddr(uint32_t blockno)
{
	if (blockno == 0 || (super && blockno >= super->s_nblocks))
		panic("bad block number %08x in diskaddr", blockno);
	return (char*) (DISKMAP + blockno * BLKSIZE);
}

// Is this virtual address mapped?
bool
va_is_mapped(void *va)
{
	return (vpd[PDX(va)] & PTE_P) && (vpt[PGNUM(va)] & PTE_P);
}

// Is this virtual address dirty?
bool
va_is_dirty(void *va)
{
	return (vpt[PGNUM(va)] & PTE_D) != 0;
}

// Fault any disk block that is read or written in to memory by
// loading it from disk.
// Hint: Use ide_read and BLKSECTS.
static void
bc_pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t blockno = ((uint32_t)addr - DISKMAP) / BLKSIZE;
	int r;

	// Check that the fault was within the block cache region
	if (addr < (void*)DISKMAP || addr >= (void*)(DISKMAP + DISKSIZE))
		panic("page fault in FS: eip %08x, va %08x, err %04x",
		      utf->utf_eip, addr, utf->utf_err);

	// Sanity check the block number.
	if (super && blockno >= super->s_nblocks)
		panic("reading non-existent block %08x\n", blockno);

#ifndef USE_EVICT_POLICY
	// Allocate a page in the disk map region, read the contents
	// of the block from the disk into that page, and mark the
	// page not-dirty (since reading the data from disk will mark
	// the page dirty).
	//
	// LAB 5: Your code here
	if ((r = sys_page_alloc(0, ROUNDDOWN(addr, PGSIZE), PTE_P | PTE_U | PTE_W)) < 0)
		panic("sys_page_alloc: %e", r);
	if ((r = ide_read(blockno * BLKSECTS, ROUNDDOWN(addr, PGSIZE), BLKSECTS)) < 0)
		panic("ide_read: %e", r);
	// Clear dirty bit
	if ((r = sys_page_map(0, ROUNDDOWN(addr, PGSIZE), 0, ROUNDDOWN(addr, PGSIZE), PTE_SYSCALL)) < 0)
		panic("sys_page_map: %e", r);
#else
	// Dispatch pgfault
	if (blockno <= 2) {
		if ((r = sys_page_alloc(0, ROUNDDOWN(addr, PGSIZE), PTE_P | PTE_U | PTE_W)) < 0)
			panic("sys_page_alloc: %e", r);
		if ((r = ide_read(blockno * BLKSECTS, ROUNDDOWN(addr, PGSIZE), BLKSECTS)) < 0)
			panic("ide_read: %e", r);
		// Clear dirty bit
		if ((r = sys_page_map(0, ROUNDDOWN(addr, PGSIZE), 0, ROUNDDOWN(addr, PGSIZE), PTE_SYSCALL)) < 0)
			panic("sys_page_map: %e", r);
	} else
		clock_bc_pgfault(ROUNDDOWN(addr, PGSIZE), blockno);
#endif

	// Check that the block we read was allocated. (exercise for
	// the reader: why do we do this *after* reading the block
	// in?)
	if (bitmap && block_is_free(blockno))
		panic("reading free block %08x\n", blockno);
}

// Flush the contents of the block containing VA out to disk if
// necessary, then clear the PTE_D bit using sys_page_map.
// If the block is not in the block cache or is not dirty, does
// nothing.
// Hint: Use va_is_mapped, va_is_dirty, and ide_write.
// Hint: Use the PTE_SYSCALL constant when calling sys_page_map.
// Hint: Don't forget to round addr down.
void
flush_block(void *addr)
{
	uint32_t blockno = ((uint32_t)addr - DISKMAP) / BLKSIZE;
	int r;

	if (addr < (void*)DISKMAP || addr >= (void*)(DISKMAP + DISKSIZE))
		panic("flush_block of bad va %08x", addr);

	// LAB 5: Your code here.
	if (!va_is_mapped(addr) || !va_is_dirty(addr))
		return;

	if ((r = ide_write(blockno * BLKSECTS, ROUNDDOWN(addr, PGSIZE), BLKSECTS)) < 0)
		panic("ide_write: %e", r);

	// Clear dirty bit
	if ((r = sys_page_map(0, ROUNDDOWN(addr, PGSIZE), 0, ROUNDDOWN(addr, PGSIZE), PTE_SYSCALL)) < 0)
		panic("sys_page_map: %e", r);
}

// Test that the block cache works, by smashing the superblock and
// reading it back.
static void
check_bc(void)
{
	struct Super backup;

	// back up super block
	memmove(&backup, diskaddr(1), sizeof backup);

	// smash it
	strcpy(diskaddr(1), "OOPS!\n");
	flush_block(diskaddr(1));
	assert(va_is_mapped(diskaddr(1)));
	assert(!va_is_dirty(diskaddr(1)));

	// clear it out
	sys_page_unmap(0, diskaddr(1));
	assert(!va_is_mapped(diskaddr(1)));

	// read it back in
	assert(strcmp(diskaddr(1), "OOPS!\n") == 0);

	// fix it
	memmove(diskaddr(1), &backup, sizeof backup);
	flush_block(diskaddr(1));

	cprintf("block cache is good\n");
}

void
bc_init(void)
{
	set_pgfault_handler(bc_pgfault);
	check_bc();
}
