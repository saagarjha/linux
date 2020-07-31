#include <asm/page.h>
#include <linux/console.h>
#include <linux/init.h>
#include <linux/memblock.h>
#include <linux/sched/task.h>
#include <linux/start_kernel.h>
#include <user/user.h>

char *empty_zero_page;

void __init setup_arch(char **cmdline_p)
{
	size_t phys_size;
	unsigned long zone_pfns[MAX_NR_ZONES] = {};

	*cmdline_p = boot_command_line;
	parse_early_param();

	phys_size = 0x10000000;
	ish_phys_base = (unsigned long) host_mmap((void *) 0x200000000, 0x10000000);
	memblock_add(__pa(ish_phys_base), phys_size);

	zone_pfns[ZONE_NORMAL] = phys_size >> PAGE_SHIFT;
	free_area_init(zone_pfns);

	empty_zero_page = memblock_alloc_low(PAGE_SIZE, PAGE_SIZE);

	min_low_pfn = 0;
	max_low_pfn = phys_size >> PAGE_SHIFT;
}


void run_kernel(void)
{
	current = &init_task;
	start_kernel();
}

#ifdef CONFIG_ISH_MAIN
int main(int argc, const char *argv[])
{
	int i;
	for (i = 1; i < argc; i++) {
		if (i > 1)
			strcat(boot_command_line, " ");
		strcat(boot_command_line, argv[i]);
	}
	run_kernel();
	return 0;
}
#endif
