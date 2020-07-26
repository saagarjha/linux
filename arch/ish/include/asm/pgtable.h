#ifndef __ASM_ISH_PGTABLE_H
#define __ASM_ISH_PGTABLE_H

#include <asm-generic/pgtable-nopud.h>

// everything else should be defined based on these
// TODO flex this a bit depending on whether the host is 64 bit?
#define _POINTER_BITS 32
#define _PGD_BITS 2
#define _PMD_BITS 9
#define _PTE_BITS 9

#define PTRS_PER_PGD	(1UL << _PGD_BITS)
#define PTRS_PER_PMD	(1UL << _PMD_BITS)
#define PTRS_PER_PTE	(1UL << _PTE_BITS)
#define PGDIR_SHIFT	(_POINTER_BITS - _PGD_BITS)
#define PGDIR_SIZE	(1UL << PGDIR_SHIFT)
#define PGDIR_MASK	(~(PGDIR_SIZE-1))
#define PMD_SHIFT	(PGDIR_SHIFT - _PMD_BITS)
#define PMD_SIZE	(1UL << PMD_SHIFT)
#define PMD_MASK	(~(PMD_SIZE-1))
#define USER_PTRS_PER_PGD ((TASK_SIZE + (PGDIR_SIZE - 1)) / PGDIR_SIZE)
#define FIRST_USER_ADDRESS 0

#if _PGD_BITS + _PMD_BITS + _PTE_BITS + PAGE_SHIFT != _POINTER_BITS
#error "pagetables don't line up!"
#endif

#define _PAGE_ACCESSED	(1 << 0)
#define _PAGE_DIRTY	(1 << 1)
#define _PAGE_WRITE	(1 << 2)
#define _PAGE_CHG_MASK	(~(_PAGE_ACCESSED | _PAGE_DIRTY | _PAGE_WRITE))

#define pte_none(x)	(pte_val(x) == 0)
#define pte_present(x)	(pte_val(x) != 0)
#define pte_young(x)	(pte_val(x) & _PAGE_ACCESSED)
#define pte_dirty(x)	(pte_val(x) & _PAGE_DIRTY)
#define pte_write(x)	(pte_val(x) & _PAGE_WRITE)
#define pte_mkold(x)	(__pte(pte_val(x) & ~_PAGE_ACCESSED))
#define pte_mkyoung(x)	(__pte(pte_val(x) | _PAGE_ACCESSED))
#define pte_mkclean(x)	(__pte(pte_val(x) & ~_PAGE_DIRTY))
#define pte_mkdirty(x)	(__pte(pte_val(x) | _PAGE_DIRTY))
#define pte_wrprotect(x)	(__pte(pte_val(x) & ~_PAGE_WRITE))
#define pte_mkwrite(x)	(__pte(pte_val(x) | _PAGE_WRITE))

#define pte_page(x)	pfn_to_page(pte_pfn(x))
#define pte_clear(mm, addr, pte)	do { pte_val(*(pte)) = 0; } while (0)

#define pmd_page(x)		virt_to_page(pmd_val(x) & PAGE_MASK)
#define pmd_page_vaddr(x)	(pmd_val(x) & _PAGE_CHG_MASK)
#define pud_page_vaddr(x)	(pud_val(x) & _PAGE_CHG_MASK)

/* appears to be garbage */
#define pmd_bad(x)	0
#define pmd_none(x)	(pmd_val(x) == 0)
#define pmd_present(x)	(pmd_val(x) != 0)
#define pmd_clear(pmd)	do { pmd_val(*(pmd)) = 0; } while (0)
#define pud_bad(x)	0
#define pud_none(x)	(pud_val(x) == 0)
#define pud_present(x)	(pud_val(x) != 0)
#define pud_clear(pud)	do { pud_val(*(pud)) = 0; } while (0)

#define mk_pte(page, pgprot)	pfn_pte(page_to_pfn(page), (pgprot))
#define pte_modify(pte, prot)	(__pte((pte_val(pte) & _PAGE_CHG_MASK) | pgprot_val(prot)))
#define pte_pfn(x)		phys_to_pfn(pte_val(x))
#define pfn_pte(pfn, pgprot)	__pte((pfn << PAGE_SHIFT) | pgprot_val(pgprot))
#define set_pte(ptep, pte)	do { *ptep = pte; } while (0)
#define set_pte_at(mm, addr, ptep, pte)	set_pte(ptep, pte)

#define pte_ERROR(e) \
	printk("%s:%d: bad pte %08lx.\n", __FILE__, __LINE__, pte_val(e))
#define pmd_ERROR(e) \
	printk("%s:%d: bad pmd %08lx.\n", __FILE__, __LINE__, pmd_val(e))
#define pgd_ERROR(e) \
	printk("%s:%d: bad pgd %08lx.\n", __FILE__, __LINE__, pgd_val(e))

#define PAGE_NONE	__pgprot(0)
#define PAGE_SHARED	__pgprot(_PAGE_WRITE)
#define PAGE_COPY	__pgprot(0)
#define PAGE_READONLY	__pgprot(0)
#define PAGE_KERNEL	__pgprot(_PAGE_WRITE)

extern pgd_t swapper_pg_dir[PTRS_PER_PGD];

#define __P000	PAGE_NONE
#define __P001	PAGE_READONLY
#define __P010	PAGE_COPY
#define __P011	PAGE_COPY
#define __P100	PAGE_READONLY
#define __P101	PAGE_READONLY
#define __P110	PAGE_COPY
#define __P111	PAGE_COPY

#define __S000	PAGE_NONE
#define __S001	PAGE_READONLY
#define __S010	PAGE_SHARED
#define __S011	PAGE_SHARED
#define __S100	PAGE_READONLY
#define __S101	PAGE_READONLY
#define __S110	PAGE_SHARED
#define __S111	PAGE_SHARED

/*
 * ZERO_PAGE is a global shared page that is always zero: used
 * for zero-mapped memory areas etc..
 */
extern char empty_zero_page[PAGE_SIZE];
#define ZERO_PAGE(vaddr) virt_to_page(empty_zero_page)

#include <asm-generic/memory_model.h>

// TODO: copied from um, don't really understand
#define __swp_type(x)			(((x).val >> 5) & 0x1f)
#define __swp_offset(x)			((x).val >> 11)
#define __swp_entry(type, offset)	((swp_entry_t) { ((type) << 5) | ((offset) << 11) })
#define __swp_entry_to_pte(x)		((pte_t) { .pte = (x).val })
#define __pte_to_swp_entry(pte)		((swp_entry_t) { pte_val(pte) })

// TODO! maybe don't
#define update_mmu_cache(vma, addr, ptep) flush_tlb_page(vma, addr)

#endif
