#include <asm/unistd.h>

/* TODO: very i386 specific */
#define __SYSCALL_I386(nr, call)	extern long call(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
#include <asm/syscalls_32.h>

#undef __SYSCALL_I386
#define __SYSCALL_I386(nr, call)	[nr] = (call),
void *sys_call_table[__NR_syscall_max+1] = {
	[0 ... __NR_syscall_max] = sys_ni_syscall,
#include <asm/syscalls_32.h>
};
