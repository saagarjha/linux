#ifndef __ASM_ISH_MMU_H
#define __ASM_ISH_MMU_H

#include <emu/emu.h>

typedef struct mm_context {
	struct emu_mm emu_mm;
	void __user *vdso;
} mm_context_t;

#endif
