#ifndef __ASM_ISH_SIGHANDLING_H
#define __ASM_ISH_SIGHANDLING_H

#include "../../../../x86/include/uapi/asm/processor-flags.h"

#define FIX_EFLAGS	(X86_EFLAGS_AC | X86_EFLAGS_OF | \
			 X86_EFLAGS_DF | X86_EFLAGS_TF | X86_EFLAGS_SF | \
			 X86_EFLAGS_ZF | X86_EFLAGS_AF | X86_EFLAGS_PF | \
			 X86_EFLAGS_CF | X86_EFLAGS_RF)

void signal_fault(struct pt_regs *regs, void __user *frame, char *where);

#endif
