#include <limits.h>
#include <poll.h>
#include <unistd.h>

#include <user/poll.h>

/* TODO use separate files */
#if HAVE_EPOLL

int real_poll_init(struct real_poll *real)
{
	real->fd = epoll_create1(0);
	if (real->fd < 0)
		return -1;
	return 0;
}

int real_poll_wait(struct real_poll *real, struct real_poll_event *events, int max, struct timespec *timeout)
{
	int timeout_millis = -1;
	if (timeout != NULL)
		timeout_millis = timeout->tv_sec * 1000 + timeout->tv_nsec / 1000000;
	return epoll_wait(real->fd, (struct epoll_event *) events, max, timeout_millis);
}

int real_poll_update(struct real_poll *real, int fd, int types, void *data)
{
	types &= ~EPOLLONESHOT;
	if (types == 0)
		return epoll_ctl(real->fd, EPOLL_CTL_DEL, fd, NULL);
	struct epoll_event epevent = {.events = types, .data.ptr = data};
	int err = epoll_ctl(real->fd, EPOLL_CTL_MOD, fd, &epevent);
	if (err < 0 && errno == ENOENT)
		err = epoll_ctl(real->fd, EPOLL_CTL_ADD, fd, &epevent);
	return err;
}

void *rpe_data(struct real_poll_event *rpe)
{
	return rpe->real.data.ptr;
}
int rpe_events(struct real_poll_event *rpe)
{
	return rpe->real.events;
}

#elif HAVE_KQUEUE

int real_poll_init(struct real_poll *real)
{
	real->fd = kqueue();
	if (real->fd < 0)
		return -1;
	return 0;
}

int real_poll_update(struct real_poll *real, int fd, int types, void *data)
{
	struct kevent e[3] = {
		{.filter = EVFILT_READ, .flags = types & (POLLIN | POLLHUP) ? EV_ADD : EV_DELETE},
		{.filter = EVFILT_WRITE, .flags = types & POLLOUT ? EV_ADD : EV_DELETE},
		{.filter = EVFILT_EXCEPT, .flags = types & POLLERR ? EV_ADD : EV_DELETE},
	};
	if (!(types & POLLIN) && types & POLLHUP) {
		// Set the low water mark really high so we'll only get woken up on a hangup
		e[0].fflags = NOTE_LOWAT;
		e[0].data = INT_MAX;
	}
	for (int i = 0; i < 3; i++) {
		e[i].ident = fd;
		e[i].udata = data;
		e[i].flags |= EV_RECEIPT;
		/*
		if (types & POLL_EDGETRIGGERED)
			e[i].flags |= EV_CLEAR;
		*/
	}

	return kevent(real->fd, e, 3, e, 3, NULL);
}

int real_poll_wait(struct real_poll *real, struct real_poll_event *events, int max, struct timespec *timeout)
{
	return kevent(real->fd, NULL, 0, (struct kevent *) events, max, timeout);
}

void *rpe_data(struct real_poll_event *rpe)
{
	return rpe->real.udata;
}
int rpe_events(struct real_poll_event *rpe)
{
	if (rpe->real.filter == EVFILT_READ) {
		int events = 0;
		if (rpe->real.data > 0)
			events |= POLLIN;
		if (rpe->real.flags & EV_EOF)
			events |= POLLHUP;
		return events;
	}
	if (rpe->real.filter == EVFILT_WRITE) return POLLOUT;
	if (rpe->real.filter == EVFILT_EXCEPT) return POLLERR;
	return 0;
}

#endif

void real_poll_close(struct real_poll *real)
{
	close(real->fd);
}

