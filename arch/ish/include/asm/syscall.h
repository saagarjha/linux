#ifndef __ASM_ISH_SYSCALL_H
#define __ASM_ISH_SYSCALL_H

extern void *sys_call_table[];

/* TODO @abi specific */
static inline int syscall_get_nr(struct task_struct *task, struct pt_regs *regs)
{
	return regs->orig_ax;
}

static inline long syscall_get_error(struct task_struct *task,
		struct pt_regs *regs)
{
	return IS_ERR_VALUE(regs->ax) ? regs->ax : 0;
}
#endif
