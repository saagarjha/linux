#ifndef __ASM_ISH_CURRENT_H
#define __ASM_ISH_CURRENT_H

/* This originally used __thread, but this breaks on Darwin: the compiler
 * assumes tlv_get_addr is a pure function, and this assumption breaks down
 * when longjmp is used to move a stack between threads. */

#if defined(__APPLE__)
typedef unsigned long __pthread_key;
#elif defined(__linux__)
typedef unsigned int __pthread_key;
#endif
extern __pthread_key __current_key;
extern void *pthread_getspecific(__pthread_key key);

static inline struct task_struct *get_current(void)
{
	return pthread_getspecific(__current_key);
}

#define current get_current()

#define in_kernel() (current != NULL)

#endif
