#include <fcntl.h>
#include <sys/mman.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

ssize_t host_write(int fd, void *data, size_t len)
{
	return write(fd, data, len);
}
ssize_t host_read(int fd, void *data, size_t len)
{
	return read(fd, data, len);
}

int fd_set_nonblock(int fd)
{
	int flags = fcntl(fd, F_GETFL);
	if (flags < 0)
		return -1;
	flags |= O_NONBLOCK;
	return fcntl(fd, F_SETFL, flags);
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
