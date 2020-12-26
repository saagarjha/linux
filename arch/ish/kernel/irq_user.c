#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include <user/fs.h>
#include <user/irq.h>
#include <user/poll.h>
#include "irq_user.h"

static pthread_t irq_thread;

/* TODO @smp: make something per-cpu */
static int irq_pending[NR_IRQS];
static pthread_mutex_t irq_lock = PTHREAD_MUTEX_INITIALIZER;

static void sigusr1_handler(int signal)
{
	pthread_mutex_lock(&irq_lock);
	for (int irq = 0; irq < NR_IRQS; irq++) {
		if (irq_pending[irq]) {
			irq_pending[irq] = 0;
			pthread_mutex_unlock(&irq_lock);
			handle_irq(irq);
			pthread_mutex_lock(&irq_lock);
		}
	}
	pthread_mutex_unlock(&irq_lock);
}

void trigger_irq(int irq)
{
	/* must not be called from a kernel thread, or may deadlock! */
	pthread_mutex_lock(&irq_lock);
	irq_pending[irq] = 1;
	pthread_mutex_unlock(&irq_lock);
	pthread_kill(irq_thread, SIGUSR1);
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
		/* TODO handle errors */
		int err = real_poll_wait(&poller, &event, 1, NULL);
		struct fd_listener *listener = rpe_data(&event);
		if (listener->data) {
			err = write(listener->pipe, &listener->data, sizeof(listener->data));
		}
		trigger_irq(listener->irq);
	}
}

void user_set_irqs_enabled(int enabled)
{
	sigset_t set;
	int err;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	err = pthread_sigmask(enabled ? SIG_UNBLOCK : SIG_BLOCK, &set, NULL);
	if (err != 0)
		panic("pthread_sigmask failed");
}

void user_init_IRQ(void)
{
	signal(SIGUSR1, sigusr1_handler);
	irq_thread = pthread_self();

	real_poll_init(&poller);
	pthread_t t;
	pthread_create(&t, NULL, poll_thread, NULL);
	pthread_detach(t);
}
