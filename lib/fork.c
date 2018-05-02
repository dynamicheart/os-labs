// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at vpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	if (!(err & FEC_WR) || 
		!(vpd[PDX((uint32_t)addr)] & PTE_P) || 
		!(vpt[PGNUM((uint32_t)addr)] & PTE_P) ||
		!(vpt[PGNUM((uint32_t)addr)] & PTE_COW)
		)
		panic("pgfault: faulting access is not write to a copy-on-write page");

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.
	//   No need to explicitly delete the old page's mapping.

	// LAB 4: Your code here.
	if ((r = sys_page_alloc(0, PFTEMP, PTE_P | PTE_U | PTE_W)) < 0)
		panic("sys_page_alloc: %e", r);
	memmove(PFTEMP, ROUNDDOWN(addr, PGSIZE), PGSIZE);
	if ((r = sys_page_map(0, PFTEMP, 0, ROUNDDOWN(addr, PGSIZE), PTE_P | PTE_U | PTE_W)) < 0)
		panic("sys_page_alloc: %e", r);
	if ((r = sys_page_unmap(0, PFTEMP)) < 0)
		panic("sys_page_unmap: %e", r);
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;
	// LAB 4: Your code here.
	int perm;
	uint32_t addr = pn * PGSIZE;

	if ((vpd[PDX(addr)] & PTE_P) && (vpt[pn] & PTE_P)) {
		if ((vpt[pn] & PTE_W) || (vpt[pn] & PTE_COW))
			perm = PTE_P | PTE_U | PTE_COW;
		else
			perm = PTE_P | PTE_U;

		if ((r = sys_page_map(0, (void *)addr, envid, (void *)addr, perm)) < 0)
			panic("sys_page_map: %e", r);

		// Remap the page copy-on-write in its own address space
		if ((vpt[pn] & PTE_W) && (r = sys_page_map(0, (void *)addr, 0, (void *)addr, PTE_P | PTE_U | PTE_COW)) < 0)
			panic("sys_page_map: %e", r);
	}

	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use vpd, vpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	envid_t envid;
	uint32_t addr;
	int r;

	set_pgfault_handler(pgfault);

	envid = sys_exofork();
	if (envid < 0)
		panic("sys_exofork: %e", envid);
	if (envid == 0) {
		// We're the child.
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}

	// We're the parent.
	for (addr = UTEXT; addr < USTACKTOP; addr += PGSIZE)
		duppage(envid, PGNUM(addr));

	// Allocate a new page for the child's user exception stack
	if ((r = sys_page_alloc(envid, (void *)(UXSTACKTOP - PGSIZE), PTE_P | PTE_U | PTE_W)) < 0)
		panic("sys_page_alloc: %e", r);

	// The parent sets the user page fault entrypoint for the child to look like its own
	extern void _pgfault_upcall(void);
	if ((r = sys_env_set_pgfault_upcall(envid, _pgfault_upcall)))
		panic("sys_env_set_pgfault_upcall: %e", r);

	// Start the child environment running
	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0)
		panic("sys_env_set_status: %e", r);

	return envid;
}

// Challenge!
int
sfork(void)
{
	envid_t envid;
	uint32_t addr;
	int r;
	// See user.ld
	extern unsigned char end[];

	set_pgfault_handler(pgfault);

	envid = sys_exofork();
	if (envid < 0)
		panic("sys_exofork: %e", envid);
	if (envid == 0) {
		// We're the child.
		// Child should not use thisenv after sfork. Use whichenv instead
		return 0;
	}

	// We're the parent.

	// User text and data - sharing page
	for (addr = UTEXT; (uint8_t *)addr < end; addr += PGSIZE) {
		if ((vpd[PDX(addr)] & PTE_P) && (vpt[PGNUM(addr)] & PTE_P)){
			int perm = vpt[PGNUM(addr)] & (PTE_P | PTE_U | PTE_W | PTE_COW | PTE_SYSCALL);
			if ((r = sys_page_map(0, (void *)addr, envid, (void *)addr, perm)) < 0)
				panic("sys_page_map: %e", r);
		}
	}

	// Stack - copy on write style
	for (; addr < USTACKTOP; addr += PGSIZE)
		duppage(envid, PGNUM(addr));

	// Allocate a new page for the child's user exception stack
	if ((r = sys_page_alloc(envid, (void *)(UXSTACKTOP - PGSIZE), PTE_P | PTE_U | PTE_W)) < 0)
		panic("sys_page_alloc: %e", r);

	// The parent sets the user page fault entrypoint for the child to look like its own
	extern void _pgfault_upcall(void);
	if ((r = sys_env_set_pgfault_upcall(envid, _pgfault_upcall)))
		panic("sys_env_set_pgfault_upcall: %e", r);

	// Start the child environment running
	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0)
		panic("sys_env_set_status: %e", r);

	return envid;
}
