#ifndef __ASM_ISH_PTRACE_H
#define __ASM_ISH_PTRACE_H

struct pt_regs {
	// TODO: fill in, maybe with struct cpu_state
};

// TODO: make these do the things they're supposed to do
#define user_mode(regs) 0
#define user_stack_pointer(regs) 0
#define instruction_pointer(regs) 0

#endif
