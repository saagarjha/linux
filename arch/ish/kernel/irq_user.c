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
static void *irq_data;
static __thread void *irq_data_for_real_this_time;
static pthread_mutex_t irq_ack_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t irq_acked = PTHREAD_COND_INITIALIZER;

static void sigusr1_handler(int signal)
{
	int irq;
	void *data;

	pthread_mutex_lock(&irq_ack_lock);
	irq = irq_num;
	data = irq_data;
	irq_num = -1;
	pthread_cond_broadcast(&irq_acked);
	pthread_mutex_unlock(&irq_ack_lock);

	irq_data_for_real_this_time = data;
	handle_irq(irq);
}

void trigger_irq(int irq, void *data)
{
	pthread_mutex_lock(&irq_ack_lock);
	irq_num = irq;
	irq_data = data;
	pthread_kill(irq_thread, SIGUSR1);
	while (irq_num != -1)
		pthread_cond_wait(&irq_acked, &irq_ack_lock);
	pthread_mutex_unlock(&irq_ack_lock);
}

static struct real_poll poller;

#define DATA_IRQ_BITS (4)
#define DATA_IRQ_SHIFT (sizeof(void *) * 8 - DATA_IRQ_BITS)
#define DATA_IRQ_MASK ~(~0ul >> DATA_IRQ_BITS)

int fd_add_irq(int fd, int types, int irq, void *data)
{
	int err;
	uintptr_t idata = (uintptr_t) data;
	if (idata & DATA_IRQ_MASK)
		__builtin_trap(); /* TODO crash different */
	idata |= (uintptr_t) irq << DATA_IRQ_SHIFT;
	err = real_poll_update(&poller, fd, types, (void *) idata);
	if (err < 0)
		return err;
	return 0;
}

static void *poll_thread(void *dummy)
{
	struct real_poll_event event;
	for (;;) {
		int err = real_poll_wait(&poller, &event, 1, NULL);
		uintptr_t idata = (uintptr_t) rpe_data(&event);
		int irq = (idata & DATA_IRQ_MASK) >> DATA_IRQ_SHIFT;
		void *data = (void *) (idata & ~DATA_IRQ_MASK);
		trigger_irq(irq, data);
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

void *current_irq_data(void)
{
	/* if (!in_irq()) panic("not in irq"); */
	return irq_data;
}
