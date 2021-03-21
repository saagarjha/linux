#define IN_ASM_OFFSETS
#include <linux/sched.h>
#include <linux/kbuild.h>

int main(void)
{
#ifdef CONFIG_SMP
	OFFSET(_TASK_CPU, task_struct, cpu);
#endif
	return 0;
}
