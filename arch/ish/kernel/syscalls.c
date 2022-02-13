#include <asm/unistd.h>

#define __SYSCALL_WITH_COMPAT(nr, native, compat)	__SYSCALL(nr, compat)

/* TODO: very i386 specific */
#define __SYSCALL(nr, call)	extern long call(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
#include <asm/syscalls_32.h>

#undef __SYSCALL
#define __SYSCALL(nr, call)	[nr] = (call),
void *sys_call_table[__NR_syscalls] = {
	[0 ... __NR_syscalls - 1] = sys_ni_syscall,
#include <asm/syscalls_32.h>
};
