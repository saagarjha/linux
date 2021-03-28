#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

#include <user/fs.h>
#include <user/irq.h>
#include <user/poll.h>
#include "irq_user.h"

extern pthread_t cpu_thread(int cpu);

void trigger_irq_check(int cpu)
{
	pthread_kill(cpu_thread(cpu), SIGUSR1);
}

static struct real_poll poller;

int fd_listen(int fd, int types, struct fd_listener *listener)
{
	int err;
	err = real_poll_update(&poller, fd, types, listener);
	if (err < 0)
		return err;
	return 0;
}

static void *poll_thread(void *dummy)
{
	struct real_poll_event event;
	for (;;) {
		int err = real_poll_wait(&poller, &event, 1, NULL);
		if (err < 0 && errno == EINTR)
			continue;
		if (err != 1) {
			panic("failed real_poll_wait: %d\n", err);
		}
		struct fd_listener *listener = rpe_data(&event);
		if (listener->data) {
			err = write(listener->pipe, &listener->data, sizeof(listener->data));
		}
		trigger_irq(listener->irq);
	}
}

static void sigusr1_handler(int signal)
{
	check_irqs();
}

void user_init_IRQ(void)
{
	signal(SIGUSR1, sigusr1_handler);

	real_poll_init(&poller);
	pthread_t t;
	pthread_create(&t, NULL, poll_thread, NULL);
	pthread_detach(t);
}

void arch_cpu_idle(void)
{
	/* This is done this way in order to be atomic.
	 * - If an IRQ arrives before enable_and_check_irqs, it will be handled
	 *   within this.
	 * - If an IRQ arrives after that, it will be handled during the call
	 *   to sigsuspend.
	 *
	 * In no case will it block indefinitely. The obvious ways of doing
	 * this, such as local_irq_enable(); pause();, have a race window
	 * between local_irq_enable() and pause() where an IRQ could be handled
	 * and pause() would still get called and block indefinitely, having
	 * missed the IRQ.
	 *
	 * x86 does sti; hlt; in asm, which at first glance seems like the same
	 * as local_irq_enable(); pause();, but in fact it is atomic because
	 * sti waits for the next instruction to complete before enabling IRQs.
	 */

	sigset_t set, old;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	pthread_sigmask(SIG_BLOCK, &set, &old);
	if (!enable_and_check_irqs())
		sigsuspend(&old);
	pthread_sigmask(SIG_SETMASK, &old, NULL);
}

void user_setup_thread(void)
{
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGPIPE);
	pthread_sigmask(SIG_BLOCK, &set, NULL);
}
