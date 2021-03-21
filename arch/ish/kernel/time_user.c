#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
#include <user/user.h>
#include <user/poll.h>
#include <user/irq.h>
#include "time_user.h"
#include "irq_user.h"

static int timer_pipe[2];

/* TODO error handling */

struct timer_msg {
	int cpu;
	int active;
	uint64_t expires;
};

static void *timer_thread(void *fuck)
{
	struct real_poll poll;
	int flags;
	struct timer {
		int active;
		uint64_t expires;
	} timers[NR_CPUS] = {};

	real_poll_init(&poll);
	real_poll_update(&poll, timer_pipe[0], POLLIN, NULL);

	flags = fcntl(timer_pipe[0], F_GETFD);
	fcntl(timer_pipe[0], F_SETFD, flags | O_NONBLOCK);

	for (;;) {
		struct timespec timeout;
		struct real_poll_event event;
		int res;
		int cpu = -1, i;
		uint64_t min_expiration = UINT64_MAX;

		for (i = 0; i < NR_CPUS; i++) {
			if (timers[i].active && timers[i].expires < min_expiration) {
				min_expiration = timers[i].expires;
				cpu = i;
			}
		}

		if (cpu != -1) {
			uint64_t now = host_unix_nanos();
			uint64_t delay = timers[cpu].expires - now;
			if ((int64_t) delay < 0) /* deal with wraparound */
				delay = 0;
			timeout.tv_nsec = delay % 1000000000;
			timeout.tv_sec = delay / 1000000000;
		}
		res = real_poll_wait(&poll, &event, 1, cpu != -1 ? &timeout : NULL);
		/* TODO: res < 0 */

		if (res > 0) {
			struct timer_msg msg;
			read(timer_pipe[0], &msg, sizeof(msg));
			timers[msg.cpu].active = msg.active;
			timers[msg.cpu].expires = msg.expires;
		} else if (res == 0) {
			trigger_irq_on_cpu(TIMER_IRQ, cpu);
			timers[cpu].active = 0;
		}
	}
}

void timer_set_next_event(int cpu, uint64_t ns)
{
	struct timer_msg msg = { .cpu = cpu,
				 .active = ns != UINT64_MAX,
				 .expires = host_unix_nanos() + ns };
	write(timer_pipe[1], &msg, sizeof(msg));
}

void timer_start_thread()
{
	/* TODO: handle error */
	pipe(timer_pipe);
	pthread_t t;
	pthread_create(&t, NULL, timer_thread, NULL);
	pthread_detach(t);
}
