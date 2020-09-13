#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
#include <user/poll.h>
#include <user/irq.h>
#include "time_user.h"

static int timer_pipe[2];
static uint64_t timer_interval;

/* TODO error handling */

static void *timer_thread(void *dummy)
{
	struct real_poll poll;
	struct real_poll_event event;
	int flags;
	int res;

	real_poll_init(&poll);
	real_poll_update(&poll, timer_pipe[0], POLLIN, NULL);

	flags = fcntl(timer_pipe[0], F_GETFD);
	fcntl(timer_pipe[0], F_SETFD, flags | O_NONBLOCK);

	for (;;) {
		struct timespec timeout;

		if (timer_interval != 0) {
			timeout.tv_nsec = timer_interval % 1000000000;
			timeout.tv_sec = timer_interval / 1000000000;
		}

		res = real_poll_wait(&poll, &event, 1, timer_interval != 0 ? &timeout : NULL);
		if (res > 0) {
			uint64_t next_interval;
			read(timer_pipe[0], &next_interval, sizeof(next_interval));
			timer_interval = next_interval;
		}
		if (res == 0) {
			trigger_irq(TIMER_IRQ);
			timer_interval = 0;
		}
	}
}

void timer_set_next_event(uint64_t ns)
{
	write(timer_pipe[1], &ns, sizeof(ns));
}

void timer_start_thread()
{
	/* TODO: handle error */
	pipe(timer_pipe);
	pthread_t t;
	pthread_create(&t, NULL, timer_thread, NULL);
	pthread_detach(t);
}
