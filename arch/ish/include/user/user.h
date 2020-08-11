#ifndef __ISH_HOST_H
#define __ISH_HOST_H

void *host_mmap(void *addr, size_t len);
uint64_t host_monotonic_nanos(void);

void walk_backtrace(void (*cb)(void *, unsigned long, char *), void *data);

void host_pause(void);

#endif
