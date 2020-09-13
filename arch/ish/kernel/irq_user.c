#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include <user/fs.h>
#include <user/irq.h>
#include <user/poll.h>

extern void handle_irq(int irq);

static pthread_t irq_thread;

/* TODO @smp: make something per-cpu */
static int irq_num = -1;
static pthread_mutex_t irq_ack_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t irq_acked = PTHREAD_COND_INITIALIZER;

static void sigusr1_handler(int signal)
{
	int irq = irq_num;
	pthread_mutex_lock(&irq_ack_lock);
	irq_num = -1;
	pthread_cond_broadcast(&irq_acked);
	pthread_mutex_unlock(&irq_ack_lock);

	handle_irq(irq);
}

void trigger_irq(int irq)
{
	irq_num = irq;

	pthread_kill(irq_thread, SIGUSR1);
	pthread_mutex_lock(&irq_ack_lock);
	while (irq_num != -1)
		pthread_cond_wait(&irq_acked, &irq_ack_lock);
	pthread_mutex_unlock(&irq_ack_lock);

	irq_num = -1;
}

static struct real_poll poller;

int fd_add_irq(int fd, int types, int irq)
{
	int err;
	if (real_poll_update(&poller, fd, types, (void *) (uintptr_t) irq) < 0)
		return -1;
	return 0;
}

static void *poll_thread(void *dummy)
{
	struct real_poll_event event;
	int err;
	for (;;) {
		err = real_poll_wait(&poller, &event, 1, NULL);
		trigger_irq((int) rpe_data(&event));
	}
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
