#include <stdlib.h>
#include <asm/processor.h>
#include <emu/emu.h>
#include <emu/exec.h>

#include "emu/cpu.h"
#include "emu/tlb.h"
#include "emu/interrupt.h"
#define ENGINE_JIT 1
#include "jit/jit.h"

extern int current_pid(void);

extern void *user_to_kernel(unsigned long virt, int is_write);
extern void handle_interrupt(int interrupt, unsigned long fault_addr, int is_write);

static __thread struct tlb the_tlb;

static void *ishemu_translate(struct mmu *mem, addr_t addr, int type)
{
	return user_to_kernel(addr, type == MEM_WRITE);
}

static struct mmu_ops ishemu_ops = {
	.translate = ishemu_translate,
};

extern unsigned long (*sys_call_table[])(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);

int emu_run_to_interrupt(struct emu *emu, struct pt_regs *regs)
{
	emu->cpu.eax = regs->ax;
	emu->cpu.ebx = regs->bx;
	emu->cpu.ecx = regs->cx;
	emu->cpu.edx = regs->dx;
	emu->cpu.esi = regs->si;
	emu->cpu.edi = regs->di;
	emu->cpu.ebp = regs->bp;
	emu->cpu.esp = regs->sp;
	emu->cpu.eip = regs->ip;

	int interrupt = cpu_run_to_interrupt(&emu->cpu, &the_tlb);

	regs->ax = emu->cpu.eax;
	regs->bx = emu->cpu.ebx;
	regs->cx = emu->cpu.ecx;
	regs->dx = emu->cpu.edx;
	regs->si = emu->cpu.esi;
	regs->di = emu->cpu.edi;
	regs->bp = emu->cpu.ebp;
	regs->sp = emu->cpu.esp;
	regs->ip = emu->cpu.eip;

	if (interrupt == INT_GPF) {
		regs->cr2 = emu->cpu.segfault_addr;
		regs->error_code = emu->cpu.segfault_was_write << 1;
	} else {
		regs->cr2 = regs->error_code = 0;
	}
	return interrupt;
}

void emu_flush_tlb(struct emu_mm *mm, unsigned long start, unsigned long end)
{
	if (the_tlb.mmu == NULL)
		return;
	tlb_flush(&the_tlb);
	jit_invalidate_range(mm->mmu.jit, start / PAGE_SIZE, (end + PAGE_SIZE - 1) / PAGE_SIZE /* TODO DIV_ROUND_UP? */);
}

void emu_mmu_init(struct emu *emu, struct emu_mm *mm)
{
	mm->mmu.jit = jit_new(&mm->mmu);
	mm->mmu.ops = &ishemu_ops;
	emu_mmu_activate(emu, mm);
}
void emu_mmu_activate(struct emu *emu, struct emu_mm *mm)
{
	emu->cpu.mmu = &mm->mmu;
}
void emu_switch_mm(struct emu *emu, struct emu_mm *mm) {
	if (mm)
		tlb_init(&the_tlb, &mm->mmu);
}
