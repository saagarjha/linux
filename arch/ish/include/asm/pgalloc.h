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
static inline pmd_t *pmd_alloc_one(struct mm_struct *mm, unsigned long address)
{
	pmd_t *pmd = (pmd_t *) __get_free_page(GFP_KERNEL | __GFP_ZERO);
	if (pmd)
		memset(pmd, 0, PAGE_SIZE);
	return pmd;
}

static inline void pgd_free(struct mm_struct *mm, pgd_t *pgd)
{
	free_page((unsigned long) pgd);
}
static inline void pmd_free(struct mm_struct *mm, pmd_t *pmd)
{
	free_page((unsigned long) pmd);
}

#endif
