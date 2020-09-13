#include <linux/console.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/sysrq.h>
#include <asm/irq.h>
#include <asm/poll.h>

#include <user/fs.h>

static struct tty_driver *stdio_driver;
struct tty_port stdio_port;
static struct tty_driver *stupid_driver;
struct tty_port stupid_port;

static int stdio_tty_open(struct tty_struct *tty, struct file *filp)
{
	return tty_port_open(&stdio_port, tty, filp);
}
static void stdio_tty_close(struct tty_struct *tty, struct file *filp)
{
	tty_port_close(&stdio_port, tty, filp);
}
static void stdio_tty_hangup(struct tty_struct *tty)
{
	tty_port_hangup(&stdio_port);
}

static int stdio_tty_write(struct tty_struct *tty, const unsigned char *data, int len)
{
	host_write(STDOUT_FD, data, len);
	return len;
}

static int stdio_tty_write_room(struct tty_struct *tty)
{
	return 4096; /* bruh we don't buffer */
}

static const struct tty_operations stdio_ops = {
	.open = stdio_tty_open,
	.close = stdio_tty_close,
	.hangup = stdio_tty_hangup,
	.write = stdio_tty_write,
	.write_room = stdio_tty_write_room,
};

static irqreturn_t stdin_irq(int irq, void *dev_id)
{
	struct tty_port *port = dev_id;
	char c, rq;
	while (host_read(STDIN_FD, &c, 1) > 0) {
		rq = 0;
		// meta => sysrq
		if (c == '\x1b') {
			if (host_read(STDIN_FD, &rq, 1) > 0) {
				if ((rq >= 'a' && rq <= 'z') || (rq >= '0' && rq <= '9')) {
					__handle_sysrq(rq, false);
					continue;
				}
			}
		}
		tty_insert_flip_char(port, c, TTY_NORMAL);
		if (rq)
			tty_insert_flip_char(port, rq, TTY_NORMAL);
	}
	tty_flip_buffer_push(port);
	return IRQ_HANDLED;
}

static int stdio_activate(struct tty_port *port, struct tty_struct *tty)
{
	if (port->client_data == NULL) {
		/* TODO: see if we can't get better error codes returned from these */
		if (fd_set_nonblock(STDIN_FD) < 0)
			return -EINVAL;
		if (fd_add_irq(STDIN_FD, POLLIN, STDIN_IRQ) < 0)
			return -EINVAL;
		port->client_data = (void *) 1;

		termio_make_raw(STDIN_FD);
	}
	return 0;
}
static const struct tty_port_operations stdio_port_ops = {
	.activate = stdio_activate,
};

static void stdio_console_write(struct console *console, const char *data, unsigned len)
{
	/* TODO this sucks */
	
	size_t i;
	for (i = 0; i < len; i++) {
		if (data[i] == '\n')
			host_write(STDOUT_FD, "\r\n", 2);
		else
			host_write(STDOUT_FD, &data[i], 1);
	}
}
static struct tty_driver *stdio_console_device(struct console *console, int *index)
{
	*index = console->index;
	return stdio_driver;
}

static struct console stdio_console = {
	.name = "tty",
	.write = stdio_console_write,
	.device = stdio_console_device,
	.flags = CON_PRINTBUFFER|CON_ANYTIME,
	.index = -1,
};

DECLARE_WAIT_QUEUE_HEAD(hang_queue);

static int stupid_open_hang(struct tty_struct *tty, struct file *filp)
{
	DEFINE_WAIT(wait);
	tty->port = &stupid_port;
	prepare_to_wait(&hang_queue, &wait, TASK_INTERRUPTIBLE);
	while (!signal_pending(current)) {
		schedule();
	}
	finish_wait(&hang_queue, &wait);
	return -EINTR;
}

static struct tty_operations stupid_ops = {
	.open = stupid_open_hang,
};

static __init int stdio_init(void)
{
	int i, err;

	tty_port_init(&stdio_port);
	stdio_port.ops = &stdio_port_ops;

	err = request_irq(STDIN_IRQ, stdin_irq, 0, "stdin", &stdio_port);
	if (err)
		pr_err("stdin: failed to request_irq: %d\n", err);

	stdio_driver = alloc_tty_driver(1); /* TODO more than 1 */
	if (!stdio_driver)
		return -ENOMEM;

	stdio_driver->driver_name = "stdio";
	stdio_driver->name = "tty";
	stdio_driver->name_base = 1;
	stdio_driver->major = TTY_MAJOR;
	stdio_driver->minor_start = 1;
	stdio_driver->type = TTY_DRIVER_TYPE_CONSOLE;
	stdio_driver->subtype = SYSTEM_TYPE_CONSOLE;
	stdio_driver->init_termios = tty_std_termios;
	stdio_driver->flags = TTY_DRIVER_REAL_RAW | TTY_DRIVER_RESET_TERMIOS;
	tty_set_operations(stdio_driver, &stdio_ops);
	tty_port_link_device(&stdio_port, stdio_driver, 0);

	if (tty_register_driver(stdio_driver))
		pr_err("stdio: failed to tty_register_driver");

	stupid_driver = alloc_tty_driver(5);
	stupid_driver->driver_name = "stupid";
	stupid_driver->name = "tty";
	stupid_driver->name_base = 2;
	stupid_driver->major = TTY_MAJOR;
	stupid_driver->minor_start = 2;
	tty_set_operations(stupid_driver, &stupid_ops);
	for (i = 0; i < 5; i++)
		tty_port_link_device(&stupid_port, stupid_driver, i);
	if (tty_register_driver(stupid_driver))
		panic("failed to register stupid driver");

	register_console(&stdio_console);
	return 0;
}
late_initcall(stdio_init);
