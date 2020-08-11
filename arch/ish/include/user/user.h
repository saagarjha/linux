#ifndef __ISH_HOST_H
#define __ISH_HOST_H

ssize_t host_write(int fd, const void *data, size_t len);
void *host_mmap(void *addr, size_t len);

void walk_backtrace(void (*cb)(void *, unsigned long, char *), void *data);

void host_pause(void);

#endif
