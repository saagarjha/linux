#ifndef __ASM_ISH_MMU_CONTEXT_H
#define __ASM_ISH_MMU_CONTEXT_H

#include <asm-generic/mm_hooks.h>
#include <emu/exec.h>

/* Dear future traveller: This is the wrong abstraction, don't feel bad about tearing it apart. */

static inline void enter_lazy_tlb(struct mm_struct *mm,
			struct task_struct *tsk)
{
}

static inline int init_new_context(struct task_struct *tsk,
			struct mm_struct *mm)
{
	emu_mmu_init(&tsk->thread.emu, &mm->context.emu_mm);
	return 0;
}

static inline void destroy_context(struct mm_struct *mm)
{
}

static inline void deactivate_mm(struct task_struct *task,
			struct mm_struct *mm)
{
	/* TODO! free jit */
}

static inline void switch_mm(struct mm_struct *prev,
			struct mm_struct *next,
			struct task_struct *tsk)
{
	emu_switch_mm(tsk ? &tsk->thread.emu : NULL, next ? &next->context.emu_mm : NULL);
}

static inline void activate_mm(struct mm_struct *prev_mm,
			       struct mm_struct *next_mm)
{
	switch_mm(prev_mm, next_mm, NULL);
}

#endif
