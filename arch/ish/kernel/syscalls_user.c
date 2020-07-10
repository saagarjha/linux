#include <unistd.h>
#include <sys/mman.h>

ssize_t host_write(int fd, void *data, size_t len)
{
	return write(fd, data, len);
}

void *host_mmap(void *addr, size_t len)
{
	return mmap(addr, len, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
}
