#ifndef __ASM_ISH_BITSPERLONG_H
#define __ASM_ISH_BITSPERLONG_H

#if defined(__x86_64__)
#define __BITS_PER_LONG 64
#else
#error "unsupported architecture"
#endif

#include <asm-generic/bitsperlong.h>

#endif
