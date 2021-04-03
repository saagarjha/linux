#include <linux/hardirq.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <asm/irqflags.h>
#include "irq_user.h"

static DEFINE_PER_CPU(int, irqs_enabled) = ARCH_IRQ_DISABLED;

unsigned long arch_local_save_flags(void)
{
	int enabled;
	if (!in_kernel())
		return 0;
	enabled = get_cpu_var(irqs_enabled);
	put_cpu_var(irqs_enabled);
	return enabled;
}

void arch_local_irq_restore(unsigned long enabled)
{
	if (!in_kernel())
		return;

	/* When this check was added, enabling interrupts would unmask SIGUSR1
	 * and interact badly with the implicit masking of SIGUSR1 on handler
	 * entry. We no longer change the signal mask, so I don't know if this
	 * is still necessary to check. But it's not breaking anything, so I
	 * see no harm in keeping it. Enabling IRQs while in an IRQ is a
	 * strange thing to do anyway. */
	BUG_ON(enabled && in_irq());

	get_cpu_var(irqs_enabled) = enabled;
	put_cpu_var(irqs_enabled);
	if (enabled)
		check_irqs();
}

/* This is used to send messages to other CPUs, so accesses should use
 * appropriate SMP barriers. */
static DEFINE_PER_CPU(unsigned long, irq_pending);

void trigger_irq_on_cpu(int irq, int cpu)
{
	/* Set bit with release semantics */
	__atomic_fetch_or(&per_cpu(irq_pending, cpu), BIT(irq), __ATOMIC_RELEASE);
	trigger_irq_check(cpu);
}

void trigger_irq(int irq)
{
	/* If it doesn't matter, just send it to CPU 0. */
	trigger_irq_on_cpu(irq, 0);
}

/* This needs to be async signal safe with itself (though not with anything else). */
int check_irqs(void)
{
	int irq;
	unsigned long *pending;
	int *enabled_irqs;
	int got_irq = 0, loop_got_irq;

	if (!in_kernel())
		/* This should only be called on kernel threads. Not using BUG
		 * because BUG only works on kernel threads anyway. */
		__builtin_trap();

	enabled_irqs = get_cpu_ptr(&irqs_enabled);
	if (!*enabled_irqs) {
		put_cpu_ptr(&irqs_enabled);
		return 0;
	}

	pending = get_cpu_ptr(&irq_pending);
	do {
		loop_got_irq = 0;
		for (irq = 0; irq < NR_IRQS; irq++) {
			static struct pt_regs dummy_irq_regs;
			struct pt_regs *old_regs;

			/* Test and clear the bit with acquire semantics. */
			unsigned long old_pending = __atomic_fetch_and(pending, ~BIT(irq), __ATOMIC_ACQUIRE);
			if (!(old_pending & BIT(irq)))
				continue;

			got_irq = loop_got_irq = 1;
			old_regs = set_irq_regs(&dummy_irq_regs);

			/* Only compiler barriers are needed here, as this only
			 * needs to synchronize with a signal handler. */
			*enabled_irqs = 0;
			barrier();
			trace_hardirqs_off();

			irq_enter();
			generic_handle_irq(irq);
			irq_exit();
			set_irq_regs(old_regs);

			trace_hardirqs_on();
			barrier();
			*enabled_irqs = 1;
		}
	} while (loop_got_irq);

	put_cpu_ptr(&irq_pending);
	put_cpu_ptr(&irqs_enabled);
	return got_irq;
}

int enable_and_check_irqs(void)
{
	int got_irq;

	get_cpu_var(irqs_enabled) = 1;
	got_irq = check_irqs();
	put_cpu_var(irqs_enabled);
	return got_irq;
}

void __init init_IRQ(void)
{
	int i;
	for (i = 0; i < NR_IRQS; i++)
		irq_set_chip_and_handler(i, &dummy_irq_chip, handle_simple_irq);
	user_init_IRQ();
}
