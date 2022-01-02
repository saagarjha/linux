#ifndef __ISH_ASM_ARCHRANDOM_H
#define __ISH_ASM_ARCHRANDOM_H

#include <linux/types.h>

extern bool __must_check arch_get_random_int(unsigned int *v);

static inline bool __must_check arch_get_random_long(unsigned long *v) {
	unsigned int i[2];
	if (!arch_get_random_int(&i[0]))
		return false;
	if (!arch_get_random_int(&i[1]))
		return false;
	*v = (unsigned long) i[1] << 32 | i[0];
	return true;
}

static inline bool __must_check arch_get_random_seed_long(unsigned long *v) {
	return false;
}

static inline bool __must_check arch_get_random_seed_int(unsigned int *v) {
	return false;
}

#endif
