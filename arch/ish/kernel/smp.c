#include <linux/smp.h>
#include <asm/smp.h>

#warning "this is fake"

int raw_smp_processor_id()
{
	// TODO return the id of the current thread
	return 0;
}

void __init smp_prepare_boot_cpu()
{
}

void __init smp_prepare_cpus(unsigned int max_cpus)
{
	// TODO start the threads
}

void __init smp_cpus_done(unsigned int max_cpus)
{
}

void smp_send_stop()
{
	// TODO kill the threads
}

void smp_send_reschedule(int cpu)
{
	panic("unimplemented %s", __func__);
}

void arch_send_call_function_ipi_mask(const struct cpumask *mask)
{
	panic("unimplemented %s", __func__);
}

void arch_send_call_function_single_ipi(int cpu)
{
	panic("unimplemented %s", __func__);
}

int __cpu_up(unsigned int cpu, struct task_struct *idle)
{
	panic("unimplemented %s", __func__);
}
