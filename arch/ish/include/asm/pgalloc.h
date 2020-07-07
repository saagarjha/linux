#ifndef __ASM_ISH_PGALLOC_H
#define __ASM_ISH_PGALLOC_H

#include <asm-generic/pgalloc.h>

// TODO: copied from um, don't really understand
static inline void pmd_populate_kernel(struct mm_struct *mm,
	pmd_t *pmd, pte_t *pte)
{
	*pmd = __pmd((unsigned long) __pa(pte));
}
static inline void pmd_populate(struct mm_struct *mm,
	pmd_t *pmd, pgtable_t pte)
{
	*pmd = __pmd(((unsigned long long)page_to_pfn(pte) <<
			(unsigned long long) PAGE_SHIFT));
}
#define __pte_free_tlb(tlb,pte, address)		\
do {							\
	pgtable_pte_page_dtor(pte);			\
	tlb_remove_page((tlb),(pte));			\
} while (0)
#define pmd_pgtable(pmd) pmd_page(pmd)

extern pgd_t *pgd_alloc(struct mm_struct *);
extern void pgd_free(struct mm_struct *mm, pgd_t *pgd);

#endif
