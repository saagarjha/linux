#ifndef __ISH_THREADS_USER_H
#define __ISH_THREADS_USER_H

struct task_struct;

int start_cpu_thread(int cpu, void (*entry)(void *), void *arg, void *stack, size_t stack_size);
#ifndef __KERNEL__
pthread_t cpu_thread(int cpu);
#endif

void setup_current(void);
void __set_current(struct task_struct *new_current);

#endif
