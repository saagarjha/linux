#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <setjmp.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <user/fs.h>
#include <user/irq.h>
#include <user/poll.h>
#include "irq_user.h"
#include "threads_user.h"

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

static struct {
	int should_wakeup;
	pthread_mutex_t lock;
	pthread_cond_t cond;
} wakeup[NR_CPUS];

void trigger_irq_check(int cpu)
{
	pthread_mutex_lock(&wakeup[cpu].lock);
	wakeup[cpu].should_wakeup = 1;
	pthread_cond_broadcast(&wakeup[cpu].cond);
	pthread_mutex_unlock(&wakeup[cpu].lock);
}

void arch_cpu_idle(void)
{
	int cpu = get_smp_processor_id();
	pthread_mutex_lock(&wakeup[cpu].lock);
	while (!wakeup[cpu].should_wakeup)
		pthread_cond_wait(&wakeup[cpu].cond, &wakeup[cpu].lock);
	wakeup[cpu].should_wakeup = 0;
	pthread_mutex_unlock(&wakeup[cpu].lock);
}

void user_init_IRQ(void)
{
	real_poll_init(&poller);
	pthread_t t;
	pthread_create(&t, NULL, poll_thread, NULL);
	pthread_detach(t);

	for (int i = 0; i < NR_CPUS; i++) {
		pthread_mutex_init(&wakeup[i].lock, NULL);
		pthread_cond_init(&wakeup[i].cond, NULL);
		wakeup[i].should_wakeup = 0;
	}
}

void user_setup_thread(void)
{
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGPIPE);
	pthread_sigmask(SIG_BLOCK, &set, NULL);
}
