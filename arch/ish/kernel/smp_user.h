#ifndef __ISH_SMP_USER_H
#define __ISH_SMP_USER_H

int start_cpu_thread(int cpu, void (*entry)(void *), void *arg, void *stack, size_t stack_size);
void user_start_secondary(void);

#endif
