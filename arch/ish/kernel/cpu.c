#include <linux/seq_file.h>

static void *c_start(struct seq_file *m, loff_t *pos)
{
	return NULL;
}

static void *c_next(struct seq_file *m, void *v, loff_t *pos)
{
	return NULL;
}

static void c_stop(struct seq_file *m, void *v)
{
}

static int c_show(struct seq_file *m, void *v)
{
	return 0;
}

const struct seq_operations cpuinfo_op = {
	.start	= c_start,
	.next	= c_next,
	.stop	= c_stop,
	.show	= c_show,
};
