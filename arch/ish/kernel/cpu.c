#include <linux/seq_file.h>
#include <linux/delay.h>

static int show_cpuinfo(struct seq_file *m, void *v)
{
	unsigned long cpu = (unsigned long) v - 1;
	seq_printf(m, "processor\t: %lu\n", cpu);
	seq_printf(m, "vendor_id\t: iSH\n");
	seq_printf(m, "bogomips\t: %lu.%02lu\n",
		   loops_per_jiffy / (500000/HZ),
		   loops_per_jiffy / (5000/HZ));
	seq_printf(m, "cache_alignment\t: %d\n", L1_CACHE_BYTES);
	seq_puts(m, "\n");
	return 0;
}

static void *c_start(struct seq_file *m, loff_t *pos)
{
	*pos = cpumask_next(*pos - 1, cpu_online_mask);
	if ((*pos) < nr_cpu_ids)
		return (void *) (uintptr_t) (*pos + 1);
	return NULL;
}

static void *c_next(struct seq_file *m, void *v, loff_t *pos)
{
	(*pos)++;
	return c_start(m, pos);
}

static void c_stop(struct seq_file *m, void *v)
{
}

const struct seq_operations cpuinfo_op = {
	.start	= c_start,
	.next	= c_next,
	.stop	= c_stop,
	.show	= show_cpuinfo,
};
