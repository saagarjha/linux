#ifndef __ASM_ISH_ARCHRANDOM_H
#define __ASM_ISH_ARCHRANDOM_H

extern uint32_t arc4random(void);

static inline bool __must_check arch_get_random_long(unsigned long *v)
{
	*v = (unsigned long) arc4random() << 32 | arc4random();
	return true;
}
static inline bool __must_check arch_get_random_int(unsigned int *v)
{
	*v = arc4random();
	return true;
}

static inline bool __must_check arch_get_random_seed_long(unsigned long *v)
{
	return false;
}
static inline bool __must_check arch_get_random_seed_int(unsigned int *v)
{
	return false;
}

#endif
