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

void noreturn emu_run(struct emu *emu)
{
	for (;;) {
		int interrupt = cpu_run_to_interrupt(&emu->cpu, &the_tlb);
		unsigned long fault_addr = emu->cpu.eip;
		int is_write = 0;
		// TODO this sucks, use pt_regs instead
		if (interrupt == INT_GPF) {
			fault_addr = emu->cpu.segfault_addr;
			is_write = emu->cpu.segfault_was_write;
		}
		if (interrupt == INT_SYSCALL) {
			int syscall = emu->cpu.eax;
			emu->cpu.eax = sys_call_table[emu->cpu.eax](emu->cpu.ebx, emu->cpu.ecx, emu->cpu.edx, emu->cpu.esi, emu->cpu.edi, emu->cpu.ebp);
			printk("%d syscall %d -> %d\n", current_pid(), syscall, emu->cpu.eax);
		}
		if (interrupt == INT_GPF) {
			/* TODO put this print somewhere better */
			printk("%d segfault at %lx ip %px sp %px\n", current_pid(), fault_addr, emu->cpu.eip, emu->cpu.esp);
		}
		handle_interrupt(interrupt, fault_addr, is_write);
	}
}

void emu_flush_tlb(struct emu_mm *mm, unsigned long start, unsigned long end)
{
	if (the_tlb.mmu == NULL)
		return;
	tlb_flush(&the_tlb);
	jit_invalidate_range(mm->mmu.jit, start / PAGE_SIZE, (end + PAGE_SIZE - 1) / PAGE_SIZE /* TODO DIV_ROUND_UP? */);
}

void emu_start_thread(struct emu *emu, unsigned long eip, unsigned long esp)
{
	emu->cpu.eip = eip;
	emu->cpu.esp = esp;
}
void emu_flush_thread(struct emu *emu)
{
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
