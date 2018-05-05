// System call stubs.

#include <inc/syscall.h>
#include <inc/lib.h>

static inline int32_t
syscall(int num, int check, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
	int32_t ret;
	/* 
	 *Note: only support 4 arguments passing. If you want to support 5
	 * arguments passing, use 'int' instead.
	 * 
	 * What is the difference between asm and __asm? 
	 *   - https://www.zhihu.com/question/53818473
	 *
	 * GCC inline assembly:
	 *   - http://www.delorie.com/djgpp/doc/brennan/brennan_att_inline_djgpp.html
	 */

	//Lab 3: Your code here

	/* 
	 * eax                      - syscall number
	 * edx, ecx, ebx, edi, %esi - arg1, arg2, arg3, arg4
	 * (ebp)                    - return pc
	 * ebp                      - return esp
	 * esp                      - trashed by sysenter
	 */
	
	asm volatile(
		// The only thing you need to save manully is ebp.
		// Esp is restored by sysexit 
		"pushl %%ebp\n\t"
		"leal after_sysenter_label%=, %%ebp\n\t"
		"pushl %%ebp\n\t"
		
		// Don't miss ',' https://stackoverflow.com/questions/11274256/inline-assembly-errors-junk-4ebp-after-register
		"movl %%esp, %%ebp\n\t"

		// Why use %= after the label?
		//  - https://stackoverflow.com/questions/31529224/inline-assembly-lable-already-defined-error
		//  - https://stackoverflow.com/questions/3898435/labels-in-gcc-inline-assembly
		//  - https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html#AssemblerTemplate
		"sysenter\n\t"
		"after_sysenter_label%=:\n\t"

		"addl $4, %%esp\n\t"
		"popl %%ebp\n\t"
		
		// Eax edx ecx ebx edi except esi are in input/out list,
		// so gcc knows to save and restore them automaticly.
		// Therefore, esi has to be listed into clobberlist
		: "=a" (ret)
		: "a" (num),
		  "d" (a1),
		  "c" (a2),
		  "b" (a3),
		  "D" (a4),
		  "S" (a5)
		: "cc", "memory");


	if(check && ret > 0)
		panic("syscall %d returned %d (> 0)", num, ret);

	return ret;
}

void
sys_cputs(const char *s, size_t len)
{
	syscall(SYS_cputs, 0, (uint32_t)s, len, 0, 0, 0);
}

int
sys_cgetc(void)
{
	return syscall(SYS_cgetc, 0, 0, 0, 0, 0, 0);
}

int
sys_env_destroy(envid_t envid)
{
	return syscall(SYS_env_destroy, 1, envid, 0, 0, 0, 0);
}

envid_t
sys_getenvid(void)
{
	 return syscall(SYS_getenvid, 0, 0, 0, 0, 0, 0);
}

int
sys_map_kernel_page(void* kpage, void* va)
{
	 return syscall(SYS_map_kernel_page, 0, (uint32_t)kpage, (uint32_t)va, 0, 0, 0);
}

void
sys_yield(void)
{
	syscall(SYS_yield, 0, 0, 0, 0, 0, 0);
}

int
sys_page_alloc(envid_t envid, void *va, int perm)
{
	return syscall(SYS_page_alloc, 1, envid, (uint32_t) va, perm, 0, 0);
}

int
sys_page_map(envid_t srcenv, void *srcva, envid_t dstenv, void *dstva, int perm)
{
	return syscall(SYS_page_map, 1, srcenv, (uint32_t) srcva, dstenv, (uint32_t) dstva, perm);
}

int
sys_page_unmap(envid_t envid, void *va)
{
	return syscall(SYS_page_unmap, 1, envid, (uint32_t) va, 0, 0, 0);
}

// sys_exofork is inlined in lib.h

int
sys_env_set_status(envid_t envid, int status)
{
	return syscall(SYS_env_set_status, 1, envid, status, 0, 0, 0);
}

int
sys_env_set_trapframe(envid_t envid, struct Trapframe *tf)
{
	return syscall(SYS_env_set_trapframe, 1, envid, (uint32_t) tf, 0, 0, 0);
}

int
sys_env_set_pgfault_upcall(envid_t envid, void *upcall)
{
	return syscall(SYS_env_set_pgfault_upcall, 1, envid, (uint32_t) upcall, 0, 0, 0);
}

int
sys_ipc_try_send(envid_t envid, uint32_t value, void *srcva, int perm)
{
	return syscall(SYS_ipc_try_send, 0, envid, value, (uint32_t) srcva, perm, 0);
}

int
sys_ipc_recv(void *dstva)
{
	return syscall(SYS_ipc_recv, 1, (uint32_t)dstva, 0, 0, 0, 0);
}

int
sys_sbrk(uint32_t inc)
{
	 return syscall(SYS_sbrk, 0, (uint32_t)inc, (uint32_t)0, 0, 0, 0);
}

