#ifndef __ASM_ISH_PTRACE_H
#define __ASM_ISH_PTRACE_H

#include <emu/emu.h>

struct pt_regs {
	struct emu emu;
};

// TODO: make these do the things they're supposed to do
#define user_mode(regs) 0
#define user_stack_pointer(regs) 0
#define instruction_pointer(regs) 0

#endif
