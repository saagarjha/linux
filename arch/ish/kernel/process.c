#include <linux/kallsyms.h>
#include <linux/ptrace.h>
#include <linux/sched.h>
#include <linux/sched/task_stack.h>
#include <linux/sched/debug.h>
#include <asm/mmu_context.h>
#include <asm/ptrace.h>
#include <asm/syscall.h>
#include <user/user.h>
#include <emu/exec.h>

#include <asm/unistd.h>

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
	regs->ip = eip;
	regs->sp = esp;
}
EXPORT_SYMBOL(start_thread);
void flush_thread(void)
{
}

/* TODO put this in a header */
extern int handle_page_fault(unsigned long address, int is_write, int *code_out);

int show_unhandled_signals = 1; /* TODO this may not be the place */
static void show_signal(struct task_struct *task, const char *desc, unsigned long addr) {
	if (show_unhandled_signals) {
		struct pt_regs *regs = task_pt_regs(task);
		printk("%s[%d] %s addr:%lx ip:%lx sp:%lx\n", task->comm,
		       task_pid_nr(task), desc, addr, regs->ip, regs->sp);
	}
}

static void __user_thread(void)
{
	struct pt_regs *regs = current_pt_regs();

	for (;;) {
		int interrupt = emu_run_to_interrupt(&current->thread.emu, current_pt_regs());
		regs->trap_nr = interrupt;
		regs->orig_ax = regs->ax;

		if (interrupt == 6) {
			/* undefined instruction */
			if (show_unhandled_signals)
				show_signal(current, "undefined instruction", regs->ip);
			force_sig_fault(SIGILL, SI_KERNEL, (void __user *) regs->ip);
		} else if (interrupt == 13 || interrupt == 14) {
			/* GPF or page fault */
			int code;
			int err = handle_page_fault(regs->cr2, regs->error_code & 2, &code);
			if (err != 0) {
				if (show_unhandled_signals)
					show_signal(current, "page fault", regs->cr2);
				force_sig_fault(SIGSEGV, code, (void __user *) regs->cr2);
			}
		} else if (interrupt == 0x80) {
			/* syscall */
			unsigned long (*syscall)(unsigned long, unsigned long,
						 unsigned long, unsigned long,
						 unsigned long, unsigned long)
				= sys_call_table[regs->orig_ax];
			if (regs->orig_ax == __NR_exit)
				printk("%s[%d] exit %d", current->comm,
				       current->pid, regs->bx);
			regs->ax = syscall(regs->bx, regs->cx, regs->dx,
					   regs->si, regs->di, regs->bp);
			printk("%s[%d] syscall %d -> %d\n", current->comm,
			       current->pid, regs->orig_ax, regs->ax);
		} else if (interrupt == 0x20) {
			/* timer */
		} else {
			if (show_unhandled_signals)
				show_signal(current, "mysterious interrupt", interrupt);
			force_sig_fault(SIGSEGV, SI_KERNEL, 0);
		}

		do_signal(regs);
	}
}

static void __kernel_thread(void)
{
	if (current->thread.request.func)
		current->thread.request.func(current->thread.request.arg);
	__user_thread();
}

int copy_thread_tls(unsigned long clone_flags, unsigned long usp,
		unsigned long arg, struct task_struct *p, unsigned long tls)
{
	// TODO: tls
	*task_pt_regs(p) = *current_pt_regs();
	task_pt_regs(p)->ax = 0;
	KSTK_ESP(p) = (unsigned long) task_stack_page(p) + THREAD_SIZE - sizeof(void *);
	KSTK_EIP(p) = (unsigned long) __kernel_thread;
	p->thread.request.func = NULL;
	if (unlikely(p->flags & PF_KTHREAD)) {
		//printk("creating kernel thread %px to call %pS with %px\n", p->comm, p, usp, arg);
		p->thread.request.func = (void (*)(void *)) usp;
		p->thread.request.arg = (void (*)(void *)) arg;
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

