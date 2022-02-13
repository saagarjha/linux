#ifndef __ASM_ISH_PGALLOC_H
#define __ASM_ISH_PGALLOC_H

#include <asm-generic/pgalloc.h>

// TODO none of this makes sense
static inline void pmd_populate_kernel(struct mm_struct *mm,
	pmd_t *pmd, pte_t *pte)
{
	*pmd = __pmd((unsigned long) pte);
}
static inline void pmd_populate(struct mm_struct *mm,
	pmd_t *pmd, pgtable_t pte)
{
	*pmd = __pmd((unsigned long) __va(page_to_phys(pte)));
}
#define pud_populate(mm, pud, pmd) do { *pud = __pud((unsigned long) pmd); } while (0)
#define __pte_free_tlb(tlb,pte, address)		\
do {							\
	pgtable_pte_page_dtor(pte);			\
	tlb_remove_page((tlb),(pte));			\
} while (0)
#define __pmd_free_tlb(tlb,x, address)   tlb_remove_page((tlb),virt_to_page(x))
#define pmd_pgtable(pmd) pmd_page(pmd)

extern pgd_t *pgd_alloc(struct mm_struct *);

#endif
