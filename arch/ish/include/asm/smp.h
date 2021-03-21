#ifndef __ISH_ASM_SMP_H
#define __ISH_ASM_SMP_H

#include <linux/cpumask.h>
#ifndef IN_ASM_OFFSETS
#include <asm/asm-offsets.h>
#endif

/* Taking the same approach as powerpc */
/* TODO: might be nice to find another way to avoid the annoyance of an asm-offsets.c */
#define raw_smp_processor_id() (*(unsigned *)((void *)current + _TASK_CPU))

void arch_send_call_function_single_ipi(int cpu);
void arch_send_call_function_ipi_mask(const struct cpumask *mask);

#endif
