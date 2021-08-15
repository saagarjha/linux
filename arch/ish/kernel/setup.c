#include <asm/page.h>
#include <linux/build_bug.h>
#include <linux/console.h>
#include <linux/init.h>
#include <linux/memblock.h>
#include <linux/sched/task.h>
#include <linux/sched/task_stack.h>
#include <linux/start_kernel.h>
#include <user/user.h>
#include <user/errno.h>
#include "irq_user.h"
#include "threads_user.h"

char *empty_zero_page;

void __init setup_arch(char **cmdline_p)
{
	unsigned long zone_pfns[MAX_NR_ZONES] = {};
	unsigned i;

	*cmdline_p = boot_command_line;
	parse_early_param();

	ish_phys_size = 0x10000000;
	ish_phys_base = (unsigned long) host_mmap((void *) 0x200000000, ish_phys_size);
	memblock_add(__pa(ish_phys_base), ish_phys_size);

	zone_pfns[ZONE_NORMAL] = ish_phys_size >> PAGE_SHIFT;
	free_area_init(zone_pfns);

	empty_zero_page = memblock_alloc_low(PAGE_SIZE, PAGE_SIZE);

	min_low_pfn = 0;
	max_low_pfn = ish_phys_size >> PAGE_SHIFT;
	max_mapnr = max_pfn = max_low_pfn;
	
	for (i = 0; i < nr_cpu_ids; i++)
		set_cpu_possible(i, true);
}

static void start_main(void *arg) {
	__set_current(arg);
	start_kernel();
}

void run_kernel(void)
{
	setup_current();
	start_cpu_thread(0, (void *) start_main, &init_task, task_stack_page(&init_task), THREAD_SIZE);
}

#ifdef CONFIG_ISH_LINK_EXECUTABLE
int main(int argc, const char *argv[])
{
	int i;
	for (i = 1; i < argc; i++) {
		if (i > 1)
			strcat(boot_command_line, " ");
		strcat(boot_command_line, argv[i]);
	}
	run_kernel();
	for (;;) host_pause();
	return 0;
}
#endif

// I see no better place to put this
#define CHECK_ERRNO(name, val) \
	static_assert(name == val);
ERRNOS(CHECK_ERRNO)
#undef CHECK_ERRNO
