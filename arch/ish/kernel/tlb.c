#include <linux/mm.h>
#include <asm/tlbflush.h>

void flush_tlb_mm(struct mm_struct *mm)
{
	__builtin_trap();
}
void flush_tlb_range(struct vm_area_struct *vma, unsigned long start, unsigned long end)
{
	__builtin_trap();
}
void flush_tlb_page(struct vm_area_struct *vma, unsigned long address)
{
	__builtin_trap();
}
void flush_tlb_kernel_range(unsigned long start, unsigned long end)
{
	__builtin_trap();
}

