#include <linux/kallsyms.h>
#include <linux/sched.h>
#include <linux/sched/task_stack.h>
#include <linux/sched/debug.h>
#include <asm/mmu_context.h>
#include <user/user.h>
#include <emu/exec.h>

void show_regs(struct pt_regs *regs)
{
	printk("<regs would go here>\n");
}

static void show_stack_frame(void *data, unsigned long addr, char *sym)
{
	printk("%s\n", sym);
}

void show_stack(struct task_struct *task, unsigned long *stack,
		const char *loglvl)
{
	pr_cont("Call Trace:\n");
	if (task != NULL && task != current)
	{
		printk("<can only dump the stack of the current task>");
		return;
	}
	walk_backtrace(show_stack_frame, (void *) loglvl);
}

void start_thread(struct pt_regs *regs, unsigned long eip, unsigned long esp)
{
	emu_start_thread(&regs->emu, eip, esp);
}
EXPORT_SYMBOL(start_thread);
void flush_thread(void)
{
	emu_flush_thread(&task_pt_regs(current)->emu);
}

static void __user_thread(void)
{
	emu_run(&task_pt_regs(current)->emu);
	// That should never return
	__builtin_trap();
}
static void __kernel_thread(void)
{
	current->thread.request.func(current->thread.request.arg);
	// If that returned, we did an execve, so go to userspace
	__user_thread();
}

int copy_thread_tls(unsigned long clone_flags, unsigned long usp,
		unsigned long arg, struct task_struct *p, unsigned long tls)
{
	// TODO: tls
	KSTK_ESP(p) = (unsigned long) task_stack_page(p) + THREAD_SIZE - sizeof(void *);
	if (unlikely(p->flags & PF_KTHREAD)) {
		KSTK_EIP(p) = (unsigned long) __kernel_thread;
		//printk("creating kernel thread %px to call %pS with %px\n", p->comm, p, usp, arg);
		p->thread.request.func = (void (*)(void *)) usp;
		p->thread.request.arg = (void (*)(void *)) arg;
	} else {
		panic("sorry, only kernel threads for now");
	}
	return 0;
}

void *__switch_to(struct task_struct *from, struct task_struct *to)
{
	struct task_struct *last;

	current = to;
	last = (void *) ksetjmp(from->thread.kernel_regs);
	if (last == NULL)
		klongjmp(to->thread.kernel_regs, (unsigned long) from);
	switch_mm(last->mm, to->mm, to);
	return last;
}

__thread struct task_struct *current;

unsigned long init_stack[THREAD_SIZE / sizeof(unsigned long)];

// fuck, this should go in ishemu
int current_pid(void)
{
	return current->pid;
}
void die(const char *message)
{
	panic(message);
}
void *mem_pt()
{
	return NULL;
}
extern void *user_to_kernel(unsigned long addr);
void *mem_ptr(void *mem, unsigned int addr, int type)
{
	return user_to_kernel(addr);
}
