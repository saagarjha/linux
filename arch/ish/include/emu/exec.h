#ifndef __ISH_EXEC_H
#define __ISH_EXEC_H

extern int emu_run_to_interrupt(struct emu *emu, struct pt_regs *regs);

extern void emu_mmu_init(struct emu *emu, struct emu_mm *mm);
extern void emu_mmu_activate(struct emu *emu, struct emu_mm *mm);
extern void emu_switch_mm(struct emu *emu, struct emu_mm *mm);

#endif
