#ifndef __ISH_HOST_H
#define __ISH_HOST_H

void *host_mmap(void *addr, size_t len);
uint64_t host_monotonic_nanos(void);
uint64_t host_unix_nanos(void);

struct sym_info {
	const char *module;
	const char *symbol;
	void *symbol_addr;
};
int lookup_symbol(void *addr, struct sym_info *out);

void host_pause(void);

#endif
