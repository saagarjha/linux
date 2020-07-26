#include <linux/ptrace.h>
#include <linux/sched.h>
#include <linux/compat.h>

void ptrace_disable(struct task_struct *child)
{
}

long arch_ptrace(struct task_struct *child, long request,
		 unsigned long addr, unsigned long data)
{
	int ret;

	switch (request) {
	default:
		ret = ptrace_request(child, request, addr, data);
		break;
	}

	return ret;
}

long compat_arch_ptrace(struct task_struct *child, compat_long_t request,
			compat_ulong_t addr, compat_ulong_t data)
{
	int ret;

	switch (request) {
	default:
		ret = ptrace_request(child, request, addr, data);
		break;
	}

	return ret;
}
