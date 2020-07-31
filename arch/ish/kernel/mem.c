#include <linux/init.h>
#include <linux/memblock.h>
#include <linux/mm.h>

unsigned long ish_phys_base;
pgd_t swapper_pg_dir[PTRS_PER_PGD];

void __init mem_init(void)
{
	memblock_free_all();
	mem_init_print_info(NULL);
}

/* Allocate and free page tables. */

pgd_t *pgd_alloc(struct mm_struct *mm)
{
	pgd_t *pgd = (pgd_t *)__get_free_page(GFP_KERNEL);

	if (pgd) {
		memset(pgd, 0, USER_PTRS_PER_PGD * sizeof(pgd_t));
		memcpy(pgd + USER_PTRS_PER_PGD,
		       swapper_pg_dir + USER_PTRS_PER_PGD,
		       (PTRS_PER_PGD - USER_PTRS_PER_PGD) * sizeof(pgd_t));
	}
	return pgd;
}
