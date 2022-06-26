#ifndef __ISH_ASM_BARRIER_H
#define __ISH_ASM_BARRIER_H

#define mb() __atomic_thread_fence(__ATOMIC_SEQ_CST)

#include <asm-generic/barrier.h>

#endif
