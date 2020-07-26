#ifndef __ASM_ISH_COMPAT_H
#define __ASM_ISH_COMPAT_H

static inline bool in_ia32_syscall()
{
	return true;
}
extern void __user *arch_compat_alloc_user_space(long len);

#include "../../../x86/include/asm/compat.h"

#endif
