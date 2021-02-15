#ifndef __ISH_HOST_H
#define __ISH_HOST_H

void *host_mmap(void *addr, size_t len);
uint64_t host_monotonic_nanos(void);
uint64_t host_unix_nanos(void);

void host_pause(void);
void host_block_sigpipe(void);

#endif
