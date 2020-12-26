#include <linux/hardirq.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <asm/irqflags.h>
#include "irq_user.h"

/* TODO @smp make this percpu */
static int irqs_enabled = ARCH_IRQ_DISABLED;

unsigned long arch_local_save_flags(void)
{
	return irqs_enabled;
}

void arch_local_irq_restore(unsigned long enabled)
{
	/* Don't try to enable interrupts while in a hardirq! This would
	 * interact badly with the signal mask changing behavior of the signal
	 * handler. */
	BUG_ON(enabled && in_irq());
	if (irqs_enabled == enabled)
		return;
	/* Interrupts should be disabled while flipping the flag. */
	if (irqs_enabled) {
		user_set_irqs_enabled(enabled);
		irqs_enabled = enabled;
	} else {
		irqs_enabled = enabled;
		user_set_irqs_enabled(enabled);
	}
}

void handle_irq(int irq)
{
	static struct pt_regs dummy_irq_regs;
	struct pt_regs *old_regs = set_irq_regs(&dummy_irq_regs);

	/* Entering a signal handler blocked the signal. Update the variable to
	 * reflect this. */
	BUG_ON(!irqs_enabled);
	irqs_enabled = 0;

	irq_enter();
	generic_handle_irq(irq);
	irq_exit();
	set_irq_regs(old_regs);

	irqs_enabled = 1;
}

void __init init_IRQ(void)
{
	int i;
	for (i = 0; i < NR_IRQS; i++)
		irq_set_chip_and_handler(i, &dummy_irq_chip, handle_simple_irq);
	user_init_IRQ();
	irqs_enabled = ARCH_IRQ_ENABLED;
}
