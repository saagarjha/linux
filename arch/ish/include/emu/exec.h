#ifndef __ISH_EXEC_H
#define __ISH_EXEC_H

extern void emu_start_thread(struct emu *emu, unsigned long eip, unsigned long esp);
extern void emu_flush_thread(struct emu *emu);

extern void __attribute__((__noreturn__)) emu_run(struct emu *emu);

extern void emu_mmu_init(struct emu *emu, struct emu_mm *mm);
extern void emu_mmu_activate(struct emu *emu, struct emu_mm *mm);
extern void emu_switch_mm(struct emu *emu, struct emu_mm *mm);

#endif
