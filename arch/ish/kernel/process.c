#include <linux/kallsyms.h>
#include <linux/sched.h>
#include <linux/sched/task_stack.h>
#include <linux/sched/debug.h>
#include <user/user.h>

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
	__builtin_trap();
}
EXPORT_SYMBOL(start_thread);
void flush_thread(void)
{
	__builtin_trap();
}

static void __thread_entry(void)
{
	current->thread.request.func(current->thread.request.arg);
}

int copy_thread(unsigned long clone_flags, unsigned long usp,
		unsigned long arg, struct task_struct *p)
{
	KSTK_ESP(p) = (unsigned long) task_stack_page(p) + THREAD_SIZE - sizeof(void *);
	KSTK_EIP(p) = (unsigned long) __thread_entry;
	if (unlikely(p->flags & PF_KTHREAD)) {
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

	//printk("switching %px -> %px\n", from, to);
	current = to;
	last = (void *) setjmp(from->thread.kernel_regs);
	if (last == NULL)
		longjmp(to->thread.kernel_regs, (unsigned long) from);
	//printk("came back to %px from %px\n", current, last);
	return last;
}

__thread struct task_struct *current;

unsigned long init_stack[THREAD_SIZE / sizeof(unsigned long)];
