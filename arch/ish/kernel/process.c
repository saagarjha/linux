#include <linux/sched.h>
#include <linux/sched/debug.h>

void show_regs(struct pt_regs *regs)
{
	__builtin_trap();
}
void show_stack(struct task_struct *task, unsigned long *stack,
		const char *loglvl)
{
	__builtin_trap();
}

void start_thread(struct pt_regs *regs, unsigned long eip, unsigned long esp)
{
	__builtin_trap();
}
EXPORT_SYMBOL(start_thread);
void flush_thread(void)
{
	__builtin_trap();
}

int copy_thread(unsigned long clone_flags, unsigned long usp,
		unsigned long arg, struct task_struct *p)
{
	__builtin_trap();
}

void *__switch_to(struct task_struct *from, struct task_struct *to)
{
	__builtin_trap();
}

__thread struct task_struct *current;

unsigned long init_stack[THREAD_SIZE / sizeof(unsigned long)];
