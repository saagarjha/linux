#include <stddef.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <user/user.h>
#include <user/errno.h>
#include <user/fs.h>

int err_map(int err)
{
	switch (err) {
#define ERRNO_CASE(name, val) \
		case name: return -val;
		ERRNOS(ERRNO_CASE)
#undef ERRNO_CASE
		default:
			return -(0x1000 | err);
	}
}
int errno_map()
{
	return err_map(errno);
}

int host_open(const char *file, int flags)
{
	int fd = open(file, flags);
	if (fd < 0)
		return errno_map();
	return fd;
}

ssize_t host_write(int fd, const void *data, size_t len)
{
	ssize_t res = write(fd, data, len);
	if (res < 0)
		return errno_map();
	return res;
}
ssize_t host_read(int fd, void *data, size_t len)
{
	ssize_t res = read(fd, data, len);
	if (res < 0)
		return errno_map();
	return res;
}
ssize_t host_pwrite(int fd, void *data, size_t len, off_t offset)
{
	ssize_t res = pwrite(fd, data, len, offset);
	if (res < 0)
		return errno_map();
	return res;
}
ssize_t host_pread(int fd, void *data, size_t len, off_t offset)
{
	ssize_t res = pread(fd, data, len, offset);
	if (res < 0)
		return errno_map();
	return res;
}

int host_close(int fd)
{
	int err = close(fd);
	if (err < 0)
		return errno_map();
	return 0;
}

int host_socket(int domain, int type, int protocol)
{
	/* I can only promise this will work on darwin and linux */
	int sock = socket(domain, type, protocol);
	if (sock < 0)
		return errno_map();
	return sock;
}

static_assert(sizeof(struct iovec) == sizeof(struct user_iovec), "iovec size");
static_assert(offsetof(struct iovec, iov_base) == offsetof(struct user_iovec, base), "iovec.iov_base");
static_assert(offsetof(struct iovec, iov_len) == offsetof(struct user_iovec, len), "iovec.iov_len");

ssize_t host_sendmsg(int fd, struct user_iovec *iov, size_t iov_len, void *name, unsigned name_len, int flags)
{
	/* need to translate flags */
	/* need to translate name */
	assert(flags == 0);
	struct msghdr msg = {
		.msg_iov = (struct iovec *) iov,
		.msg_iovlen = iov_len,
		.msg_name = name,
		.msg_namelen = name_len,
	};
	ssize_t len = sendmsg(fd, &msg, flags);
	if (len < 0)
		return errno_map();
	return len;
}

ssize_t host_recvmsg(int fd, struct user_iovec *iov, size_t iov_len, void *name, int *name_len_out, int *flags_out, int flags)
{
	assert(flags == 0);
	struct msghdr msg = {
		.msg_iov = (struct iovec *) iov,
		.msg_iovlen = iov_len,
		.msg_name = name,
		.msg_namelen = 128, /* TODO do better */
	};
	ssize_t len = recvmsg(fd, &msg, flags);
	if (len < 0)
		return errno_map();
	*name_len_out = msg.msg_namelen;
	assert(msg.msg_flags == 0);
	*flags_out = msg.msg_flags;
	return len;
}

int fd_set_nonblock(int fd)
{
	int flags = fcntl(fd, F_GETFL);
	if (flags < 0)
		return errno_map();
	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0)
		return errno_map();
	return 0;
}

int fd_poll(int fd)
{
	/* This is dramatically simplified by assuming fd will always be a
	 * socket, and ignoring any Darwin idiosyncracies with other types of
	 * fds. */
	struct pollfd p = {.fd = fd, .events = POLLPRI};
	/* attempt to prevent POLLNVAL */
	int flags = fcntl(fd, F_GETFL, 0);
	if ((flags & O_ACCMODE) != O_WRONLY)
		p.events |= POLLIN;
	if ((flags & O_ACCMODE) != O_RDONLY)
		p.events |= POLLOUT;
	if (poll(&p, 1, 0) <= 0)
		return 0;
	return p.revents;
}
void termio_make_raw(int fd)
{
	struct termios tio;
	tcgetattr(fd, &tio);
	cfmakeraw(&tio);
	tcsetattr(fd, TCSANOW, &tio);
}

void host_pause(void)
{
	pause();
}

void *host_mmap(void *addr, size_t len)
{
	return mmap(addr, len, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
}

uint64_t host_monotonic_nanos(void)
{
#ifdef __APPLE__
	return clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
#endif
#ifdef __linux__
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000000 + ts.tv_nsec;
#endif
}

uint64_t host_unix_nanos(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return ts.tv_sec * 1000000000 + ts.tv_nsec;
}
