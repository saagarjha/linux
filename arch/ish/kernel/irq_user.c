/* jank ass pile of garbage */
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/event.h>
#include <unistd.h>
#include <user/fs.h>

static int poller;
static pthread_t irq_thread;

static int poke;
static pthread_mutex_t poke_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t poked = PTHREAD_COND_INITIALIZER;

struct fd_callback {
	fd_callback_t cb;
	void *data;
};

int fd_add_listener(int fd, int types, fd_callback_t callback, void *data)
{
	int err;
	int flags;
	struct fd_callback *cb;
	struct kevent event;

	flags = fcntl(fd, F_GETFL);
	if (flags < 0)
		return -1;
	err = fcntl(fd, F_SETFL, flags | O_ASYNC);
	if (err < 0)
		return -1;

	cb = calloc(1, sizeof(struct fd_callback));
	cb->cb = callback;
	cb->data = data;
	event.ident = fd;
	event.filter = EVFILT_READ;
	event.flags = EV_ADD;
	event.udata = cb;
	err = kevent(poller, &event, 1, NULL, 0, NULL);
	if (err < 0) {
		free(cb);
		return -1;
	}
	return 0;
}

static void sigio_handler(int sigio)
{
	struct kevent ev;
	struct timespec zero = {};
	int err;
	while ((err = kevent(poller, NULL, 0, &ev, 1, &zero)) > 0) {
		struct fd_callback *cb = ev.udata;
		cb->cb(ev.ident, LISTEN_READ, cb->data);
	}

	/* ack the poke */
	pthread_mutex_lock(&poke_lock);
	poke = 0;
	pthread_cond_broadcast(&poked);
	pthread_mutex_unlock(&poke_lock);
}

static void *poll_to_sigio_thread(void *null)
{
	sigset_t all_sigs;
	sigfillset(&all_sigs);
	pthread_sigmask(SIG_BLOCK, &all_sigs, NULL);
	for (;;) {
		struct kevent dummy;
		kevent(poller, NULL, 0, &dummy, 1, NULL);
		pthread_mutex_lock(&poke_lock);
		poke = 1;
		pthread_kill(irq_thread, SIGIO);
		while (poke)
			pthread_cond_wait(&poked, &poke_lock);
		pthread_mutex_unlock(&poke_lock);
	}
}

void user_init_IRQ(void)
{
	signal(SIGIO, sigio_handler);
	irq_thread = pthread_self();
	poller = kqueue();
	pthread_t t;
	pthread_create(&t, NULL, poll_to_sigio_thread, NULL);
	pthread_detach(t);
}
