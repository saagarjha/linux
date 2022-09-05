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
#include <emu/exec.h>
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

static int running_under_rr;

void trigger_irq_check(int cpu)
{
	pthread_mutex_lock(&wakeup[cpu].lock);
	__atomic_store_n(&wakeup[cpu].should_wakeup, 1, __ATOMIC_SEQ_CST);
	pthread_cond_broadcast(&wakeup[cpu].cond);
	pthread_mutex_unlock(&wakeup[cpu].lock);
	emu_poke_cpu(cpu);
}

void arch_cpu_idle(void)
{
	int cpu = get_smp_processor_id();
	pthread_mutex_lock(&wakeup[cpu].lock);
	while (!__atomic_exchange_n(&wakeup[cpu].should_wakeup, 0, __ATOMIC_SEQ_CST))
		pthread_cond_wait(&wakeup[cpu].cond, &wakeup[cpu].lock);
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

	if (getenv("RUNNING_UNDER_RR"))
		running_under_rr = 1;
}

void user_setup_thread(void)
{
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGPIPE);
	pthread_sigmask(SIG_BLOCK, &set, NULL);
}

void cpu_relax_user()
{
	/* In order to deterministically record, rr needs to run every thread
	 * on one cpu. Many bugs won't show up with only one cpu, so we often
	 * have to pretend there are multiple cpus. But this is extremely
	 * unfriendly to spinlocks. With actual concurrency, a spinlock release
	 * will be seen by other cpus relatively quickly, but under rr's
	 * scheduler, it will take until the end of the quantum. So, if running
	 * under rr, tell rr that now would be a good time to try and run
	 * another thread. We don't want to do this if not under rr, as this
	 * would unfairly deprioritize us. */
	if (running_under_rr)
		sched_yield();
}
