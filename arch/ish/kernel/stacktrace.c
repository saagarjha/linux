#include <linux/sched.h>
#include <linux/sched/task_stack.h>
#include <linux/stacktrace.h>

typedef void (*frame_cb_t)(void *data, unsigned long addr, int reliable);

void walk_stack(struct task_struct *task, frame_cb_t cb, void *data)
{
	struct stackframe {
		struct stackframe *fp;
		void *ret;
	};

	struct stackframe *frame;
	void *pc;
	void *stack_low, *stack_high;

	if (task == NULL)
		task = current;
	stack_low = task_stack_page(task);
	stack_high = stack_low + THREAD_SIZE;

	if (task == current) {
		frame = __builtin_frame_address(0);
		pc = 0;
	} else {
		frame = (void *)task->thread.kernel_regs->rbp;
		pc = (void *)KSTK_EIP(task);
	}

	for (;;) {
		if (pc)
			cb(data, (unsigned long)pc, 1);
		if ((void *)frame < stack_low || (void *)frame >= stack_high)
			break;
		pc = frame->ret;
		frame = frame->fp;
	}
}

static void show_stack_frame(void *data, unsigned long addr, int reliable)
{
	const char *loglvl = data;
	printk("%s [<%px>] %s%pB\n", loglvl, (void *)addr, reliable ? "" : "? ",
	       (void *)addr);
}

void show_stack(struct task_struct *task, unsigned long *stack,
		const char *loglvl)
{
	pr_cont("Call Trace:\n");
	walk_stack(task, show_stack_frame, (void *)loglvl);
}

#ifdef CONFIG_STACKTRACE
void save_stack_frame(void *data, unsigned long addr, int reliable)
{
	struct stack_trace *trace = data;
	if (!reliable)
		return;
	if (trace->nr_entries >= trace->max_entries)
		return;
	trace->entries[trace->nr_entries++] = addr;
}

void save_stack_trace(struct stack_trace *trace)
{
	walk_stack(current, save_stack_frame, trace);
}
#endif
