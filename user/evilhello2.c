// evil hello world -- kernel pointer passed to kernel
// kernel should destroy user environment in response

#include <inc/lib.h>
#include <inc/mmu.h>
#include <inc/x86.h>

struct Segdesc *uaddr_gdt;
struct Segdesc seg_backup;

// Call this function with ring0 privilege
void evil()
{
	// Kernel memory access
	*(char*)0xf010000a = 0;

	// Out put something via outb
	outb(0x3f8, 'I');
	outb(0x3f8, 'N');
	outb(0x3f8, ' ');
	outb(0x3f8, 'R');
	outb(0x3f8, 'I');
	outb(0x3f8, 'N');
	outb(0x3f8, 'G');
	outb(0x3f8, '0');
	outb(0x3f8, '!');
	outb(0x3f8, '!');
	outb(0x3f8, '!');
	outb(0x3f8, '\n');
}

void fun_wrapper()
{
    evil();
    uaddr_gdt[GD_TSS0 >> 3] = seg_backup;
    asm volatile("leave"::);
    asm volatile("lret"::);
}

static void
sgdt(struct Pseudodesc* gdtd)
{
	__asm __volatile("sgdt %0" :  "=m" (*gdtd));
}

// Invoke a given function pointer with ring0 privilege, then return to ring3
void ring0_call(void (*fun_ptr)(void)) {
    // Here's some hints on how to achieve this.
    // 1. Store the GDT descripter to memory (sgdt instruction)
    // 2. Map GDT in user space (sys_map_kernel_page)
    // 3. Setup a CALLGATE in GDT (SETCALLGATE macro)
    // 4. Enter ring0 (lcall instruction)
    // 5. Call the function pointer
    // 6. Recover GDT entry modified in step 3 (if any)
    // 7. Leave ring0 (lret instruction)

    // Hint : use a wrapper function to call fun_ptr. Feel free
    //        to add any functions or global variables in this 
    //        file if necessary.

    // Lab3 : Your Code Here
    // 1. Read gdtd
    struct Pseudodesc gdtd;
    sgdt(&gdtd);
    // 2. Map gdt in user address space
    uaddr_gdt = (struct Segdesc*)ROUNDUP(sys_sbrk(0), PGSIZE);
    sys_map_kernel_page((void *)(gdtd.pd_base), (void*)uaddr_gdt);
    uaddr_gdt = (struct Segdesc *)((void*)uaddr_gdt + PGOFF(gdtd.pd_base));

    // 3. Setup callgate, GD_UD and GD_UT are also OK
    seg_backup = uaddr_gdt[GD_TSS0 >> 3];

    SETCALLGATE(*(struct Gatedesc*)&uaddr_gdt[GD_TSS0 >> 3], GD_KT, fun_wrapper, 0x3);

    asm volatile("lcall %0, $0"::"i"(GD_TSS0));
}

void
umain(int argc, char **argv)
{
    // call the evil function in ring0
	ring0_call(&evil);

	// call the evil function in ring3
	evil();
}

