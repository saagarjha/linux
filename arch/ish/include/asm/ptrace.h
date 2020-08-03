#ifndef __ASM_ISH_PTRACE_H
#define __ASM_ISH_PTRACE_H

struct pt_regs {
	unsigned long ax;
	unsigned long bx;
	unsigned long cx;
	unsigned long dx;
	unsigned long si;
	unsigned long di;
	unsigned long bp;
	unsigned long sp;
	unsigned long ip;
	unsigned long flags;

	unsigned long orig_ax;

	unsigned long cr2;
	unsigned long trap_nr;
	unsigned long error_code;
};

#define user_mode(regs) 0 /* TODO */
#define user_stack_pointer(regs) ((regs)->sp)
#define instruction_pointer(regs) ((regs)->ip)

#endif
