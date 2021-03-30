#ifndef __ASM_ISH_CURRENT_H
#define __ASM_ISH_CURRENT_H

#include <linux/compiler.h>

/* This originally used __thread, but this breaks on Darwin: the compiler
 * assumes tlv_get_addr is a pure function, and this assumption breaks down
 * when longjmp is used to move a stack between threads. */

extern unsigned long __current_key;
extern void *pthread_getspecific(unsigned long key);

static __always_inline struct task_struct *get_current(void)
{
	return pthread_getspecific(__current_key);
}

#define current get_current()

#define in_kernel() (current != NULL)

#endif
