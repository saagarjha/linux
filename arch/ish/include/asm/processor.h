#ifndef __ASM_ISH_PROCESSOR_H
#define __ASM_ISH_PROCESSOR_H

#include <asm/ptrace.h>
#include <emu/emu.h>
#include <user/setjmp.h>

struct task_struct;

static inline void cpu_relax(void)
{
	// TODO: asm("pause")
}

static inline unsigned long get_wchan(struct task_struct *p)
{
	// TODO
	return 0;
}

static inline void release_thread(struct task_struct *task)
{
}

struct thread_struct {
	struct emu emu;
	struct pt_regs regs;
	kjmp_buf kernel_regs;
	struct {
		void (*func)(void *);
		void *arg;
	} request;
};
#define INIT_THREAD \
{ \
}

#define TASK_SIZE 0xffffe000

/*
 * This decides where the kernel will search for a free chunk of vm
 * space during mmap's.
 */
#define TASK_UNMAPPED_BASE	(PAGE_ALIGN(TASK_SIZE / 3))

#define KSTK_ESP(t)	((t)->thread.kernel_regs->jb_sp)
#define KSTK_EIP(t)	((t)->thread.kernel_regs->jb_ip)
#define task_pt_regs(t)	(&(t)->thread.regs)

typedef struct {
	unsigned long seg;
} mm_segment_t;

/* Do necessary setup to start up a newly executed thread. */
extern void start_thread(struct pt_regs *regs, unsigned long entry, 
			 unsigned long stack);

#define STACK_TOP	TASK_SIZE
#define STACK_TOP_MAX	STACK_TOP

#endif
