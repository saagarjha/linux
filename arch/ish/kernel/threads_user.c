#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <user/errno.h>
#include "threads_user.h"
#include <asm/current.h>

static int cpu_thread_started[NR_CPUS];
static pthread_t cpu_threads[NR_CPUS];

int start_cpu_thread(int cpu, void (*entry)(void *), void *arg, void *stack, size_t stack_size)
{
	pthread_attr_t attr;

	assert(cpu <= NR_CPUS);
	if (cpu_thread_started[cpu])
		return err_map(EINVAL);

	pthread_attr_init(&attr);
	pthread_attr_setstack(&attr, stack, stack_size);
	int err = pthread_create(&cpu_threads[cpu], &attr, (void*(*)(void*)) entry, arg);
	cpu_thread_started[cpu] = 1;
	if (err < 0)
		return err_map(err);
	return 0;
}

pthread_t cpu_thread(int cpu)
{
	assert(cpu <= NR_CPUS);
	return cpu_threads[cpu];
}

void smp_setup_processor_id(void)
{
	cpu_threads[0] = pthread_self();
}

__pthread_key __current_key;
static_assert(sizeof(__current_key) == sizeof(pthread_key_t), "");

void setup_current(void)
{
	pthread_key_create(&__current_key, NULL);
}

void __set_current(struct task_struct *new_current)
{
	pthread_setspecific(__current_key, new_current);
}
