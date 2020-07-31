#include <linux/mm.h>
#include <asm/tlbflush.h>
#include <asm/processor.h>

#ifdef CONFIG_SMP
#error "still need to figure out SMP"
#endif

extern void emu_flush_tlb(struct emu_mm *mm, unsigned long start, unsigned long end);

static void flush_tlb_mm_range(struct mm_struct *mm, unsigned long start, unsigned long end)
{
	emu_flush_tlb(&mm->context.emu_mm, start, end);
}
void flush_tlb_mm(struct mm_struct *mm)
{
	flush_tlb_mm_range(mm, 0, TASK_SIZE);
}
void flush_tlb_range(struct vm_area_struct *vma, unsigned long start, unsigned long end)
{
	flush_tlb_mm_range(vma->vm_mm, start, end);
}
void flush_tlb_page(struct vm_area_struct *vma, unsigned long address)
{
	flush_tlb_range(vma, address, address + 1);
}
