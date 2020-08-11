#include <linux/console.h>
#include <linux/init.h>

#include <user/fs.h>

static void early_console_write(struct console *con, const char *s, unsigned int n)
{
	host_write(2, s, n);
}

static struct console early_console_dev = {
	.name = "earlycon",
	.write = early_console_write,
	.flags = CON_BOOT,
	.index = -1,
};

static int __init setup_early_printk(char *buf)
{
	if (!early_console) {
		early_console = &early_console_dev;
		register_console(&early_console_dev);
	}
	return 0;
}

early_param("earlyprintk", setup_early_printk);
