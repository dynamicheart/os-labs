// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>
#include <kern/trap.h>
#include <kern/pmap.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "backtrace", "Print a backtrace of the stack", mon_backtrace },
	{ "time", "Count a program's running time", mon_time },
	{ "showmappings", "Display physical page mappings", mon_showmappings},
	{ "setpermission", "Change the permissions of any mapping", mon_setpermission},
	{ "memdump", "Dump the contents of a range of memory", mon_memdump}
};
#define NCOMMANDS (sizeof(commands)/sizeof(commands[0]))

unsigned read_eip();

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < NCOMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		(end-entry+1023)/1024);
	return 0;
}

// Lab1 only
// read the pointer to the retaddr on the stack
static uint32_t
read_pretaddr() {
    uint32_t pretaddr;
    __asm __volatile("leal 4(%%ebp), %0" : "=r" (pretaddr));
    return pretaddr;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	// Your code here.
	uint32_t eip = read_eip(), ebp = read_ebp();
	uint32_t args[5] = {0};
	struct Eipdebuginfo info;

	cprintf("Stack backtrace:\n");

	while(ebp != 0) {
		for(uint32_t i = 0; i < 5; i++) {
			args[i] = *((uint32_t *)(ebp + 8 + 4 * i));
		}
		cprintf("  eip %08x ebp %08x args %08x %08x %08x %08x %08x\n", eip, ebp, args[0], args[1], args[2], args[3], args[4]);
		if(debuginfo_eip(eip, &info) == 0){
			cprintf("	 %s:%d: ", info.eip_file, info.eip_line);
			for(int i = 0; i < info.eip_fn_namelen; i++) cprintf("%c", info.eip_fn_name[i]);
			cprintf("+%d\n", eip - info.eip_fn_addr);
		}

		ebp = *((uint32_t *)ebp);
		eip = *((uint32_t *)(ebp + 4));
	}

	return 0;
}

int
mon_time(int argc, char **argv, struct Trapframe *tf)
{
	if (argc < 2) {
		cprintf("Usage: time [command]\n");
		return 0;
	}

	uint32_t lo, hi;
	uint64_t start = 0, end = 0;
	for (int i = 0; i < NCOMMANDS; i++) {
		if (strcmp(argv[1], commands[i].name) == 0 && strcmp(argv[1], "time") != 0){
			__asm __volatile("rdtsc":"=a"(lo),"=d"(hi));
			start = (uint64_t)hi << 32 | lo;
			commands[i].func(argc - 1, argv + 1, tf);
			__asm __volatile("rdtsc":"=a"(lo),"=d"(hi));
			end = (uint64_t)hi << 32 | lo;

			cprintf("%s cycles: %d\n", commands[i].name, end - start);
			return 0;
		} else if(strcmp(commands[i].name, "time") == 0 && strcmp(argv[1], "time") == 0){
			// Multiple time commands act like one time command
			return commands[i].func(argc - 1, argv + 1, tf);
		}
	}

	cprintf("Unknown command:'%s'\n\n", argv[1]);
	return 0;
}

int
mon_showmappings(int argc, char **argv, struct Trapframe *tf)
{
	if (argc != 3) {
		cprintf("Usage: showmappings [low_virtual_address] [high_virtual_address]\n");
		return 0;
	}

	uint32_t low_vaddress, high_vaddress;
	char *end1 = NULL, *end2 = NULL;
	pte_t *pte;

	// May be overflow here
	low_vaddress = (uint32_t)strtol(argv[1], &end1, 0);
	high_vaddress = (uint32_t)strtol(argv[2], &end2, 0);

	if (end1 != argv[1] + strlen(argv[1]) || end2 != argv[2] + strlen(argv[2])) {
		cprintf("Invalid virtual address\n");
		return 0;
	}

	if (low_vaddress > high_vaddress) {
		cprintf("Invalid virtual address ranges\n");
		return 0;
	}

	low_vaddress = ROUNDDOWN(low_vaddress, PGSIZE); 
	high_vaddress = ROUNDDOWN(high_vaddress, PGSIZE);

	while (1){
		pte = pgdir_walk(kern_pgdir, (void *)low_vaddress, 0);
		if (pte && (*pte & PTE_P))
			cprintf("0x%08x ---> 0x%08x   %c%c%c%c%c%c%c%c%c\n",
				low_vaddress,
				*pte & PTE_PS ? (uint32_t)(*pte & (~0x3FF)) : (uint32_t)(*pte & (~0x2FFFFF)),
				*pte & PTE_P ? 'P' : '-',
				*pte & PTE_W ? 'W' : '-',
				*pte & PTE_U ? 'U' : '-',
				*pte & PTE_PWT ? 'T' : '-',
				*pte & PTE_PCD ? 'C' : '-',
				*pte & PTE_A ? 'A' : '-',
				*pte & PTE_D ? 'D' : '-',
				*pte & PTE_PS ? 'S' : '-',
				*pte & PTE_G ? 'G' : '-'
				);

		// Large page
		if (pte && (*pte & (PTE_P | PTE_PS))) {
			if (low_vaddress == 0xFFC00000)
				break;
			low_vaddress += PTSIZE;
		} else {
			if (low_vaddress == 0xFFFFFC00)
				break;
			low_vaddress += PGSIZE;
		}

		if (low_vaddress > high_vaddress)
			break;
	}

	return 0;
}

int
mon_setpermission(int argc, char **argv, struct Trapframe *tf)
{
	if (argc != 3) {
		cprintf("Usage: setpermission [virtual_address] [permissions]\n");
		return 0;
	}

	char *end = NULL;
	uint32_t vaddress, permission;
	pte_t *pte;

	// May be overflow here
	vaddress = ROUNDDOWN((uint32_t) strtol(argv[1], &end, 0), PGSIZE);
	permission = ((uint32_t) strtol(argv[1], &end, 0)) & 0x1FF;

	pte = pgdir_walk(kern_pgdir, (void *)vaddress, 0);

	if (pte && (*pte & PTE_P)) {
		cprintf("BEFORE: 0x%08x ---> 0x%08x  %c%c%c%c%c%c%c%c%c\n",
				vaddress, 
				(uint32_t)(*pte & (~0x3FF)),
				*pte & PTE_P ? 'P' : '-',
				*pte & PTE_W ? 'W' : '-',
				*pte & PTE_U ? 'U' : '-',
				*pte & PTE_PWT ? 'T' : '-',
				*pte & PTE_PCD ? 'C' : '-',
				*pte & PTE_A ? 'A' : '-',
				*pte & PTE_D ? 'D' : '-',
				*pte & PTE_PS ? 'S' : '-',
				*pte & PTE_G ? 'G' : '-'
				);

		*pte = *pte & (permission | ~0x1FF);

		cprintf("AFTER: 0x%08x ---> 0x%08x  %c%c%c%c%c%c%c%c%c\n",
				vaddress, 
				(uint32_t)(*pte & (~0x3FF)),
				*pte & PTE_P ? 'P' : '-',
				*pte & PTE_W ? 'W' : '-',
				*pte & PTE_U ? 'U' : '-',
				*pte & PTE_PWT ? 'T' : '-',
				*pte & PTE_PCD ? 'C' : '-',
				*pte & PTE_A ? 'A' : '-',
				*pte & PTE_D ? 'D' : '-',
				*pte & PTE_PS ? 'S' : '-',
				*pte & PTE_G ? 'G' : '-'
				);
	}
	else 
		cprintf("Page not existed\n");

	return 0;
}

int
mon_memdump(int argc, char **argv, struct Trapframe *tf)
{
	if (argc != 4) {
		cprintf("Usage: memdump [-v/p] [low_address] [high_address]\n");
		return 0;
	}

	// Is the address virtual?
	int vtype = 1;

	if (strcmp(argv[1], "-v") == 0)
		vtype = 1;
	else if (strcmp(argv[1], "-p") == 0)
		vtype = 0;
	else {
		cprintf("Usage: memdump [-v/p] [low_address] [high_address]\n");
		return 0;
	}

	uint32_t low_vaddress, high_vaddress, next_page_addr;
	char *end2 = NULL, *end3 = NULL;
	pte_t *pte;

	// May be overflow here
	low_vaddress = (uint32_t)strtol(argv[2], &end2, 0);
	high_vaddress = (uint32_t)strtol(argv[3], &end3, 0);

	if (!vtype) {
		if (high_vaddress >= 0xFFFFFFFF - KERNBASE) {
			cprintf("Invalid physical address\n");
			return 0;
		}

		low_vaddress += KERNBASE;
		high_vaddress += KERNBASE;
	}

	if (end2 != argv[2] + strlen(argv[2]) || end3 != argv[3] + strlen(argv[3])) {
		cprintf("Invalid virtual address\n");
		return 0;
	}

	if (low_vaddress > high_vaddress) {
		cprintf("Invalid virtual address ranges\n");
		return 0;
	}

	while (low_vaddress <= high_vaddress){
		pte = pgdir_walk(kern_pgdir, (void *)low_vaddress, 0);
		if (pte && (*pte & PTE_P) && (*pte & PTE_PS)) {
			next_page_addr = ROUNDUP(low_vaddress + 1, PTSIZE);
			while (low_vaddress < next_page_addr && low_vaddress <= high_vaddress){
				// Little endian
				cprintf("%08x: %02x %02x %02x %02x\n",
					low_vaddress,
					*((unsigned char *)low_vaddress),
					*(((unsigned char *)low_vaddress) + 1),
					*(((unsigned char *)low_vaddress) + 2),
					*(((unsigned char *)low_vaddress) + 3)
					);
				low_vaddress++;
			}
			low_vaddress = next_page_addr;
		} else if (pte && (*pte & PTE_P)) {
			next_page_addr = ROUNDUP(low_vaddress + 1, PGSIZE);
			while (low_vaddress < next_page_addr && low_vaddress <= high_vaddress) {
				// Little endian
				cprintf("%08x: %02x %02x %02x %02x\n",
					low_vaddress,
					*((unsigned char *)low_vaddress),
					*(((unsigned char *)low_vaddress) + 1),
					*(((unsigned char *)low_vaddress) + 2),
					*(((unsigned char *)low_vaddress) + 3)
					);
				low_vaddress++;
			}
			low_vaddress = next_page_addr;
		} else{
			next_page_addr = ROUNDUP(low_vaddress + 1, PGSIZE);
			low_vaddress = next_page_addr;
		}
	}

	return 0;
}

/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < NCOMMANDS; i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");

	if (tf != NULL)
		print_trapframe(tf);

	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}

// return EIP of caller.
// does not work if inlined.
// putting at the end of the file seems to prevent inlining.
unsigned
read_eip()
{
	uint32_t callerpc;
	__asm __volatile("movl 4(%%ebp), %0" : "=r" (callerpc));
	return callerpc;
}
