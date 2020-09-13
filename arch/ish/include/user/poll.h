/* Platform-independent epoll. Originally copied from fs/poll.c in iSH. */
#ifndef __ISH_USER_POLL_H
#define __ISH_USER_POLL_H

#include <poll.h>

#if defined(__linux__)
#include <sys/epoll.h>
#define HAVE_EPOLL 1
#elif defined(__APPLE__)
#include <sys/event.h>
#define HAVE_KQUEUE 1
#endif

struct real_poll {
    int fd;
};

struct real_poll_event {
#if defined(HAVE_EPOLL)
	struct epoll_event real;
#elif defined(HAVE_KQUEUE)
	struct kevent real;
#endif
};

int real_poll_init(struct real_poll *real);
void real_poll_close(struct real_poll *real);

int real_poll_wait(struct real_poll *real, struct real_poll_event *events, int max, struct timespec *timeout);
int real_poll_update(struct real_poll *real, int fd, int types, void *data);

void *rpe_data(struct real_poll_event *rpe);
int rpe_events(struct real_poll_event *rpe);

#endif
