#include <linux/hardirq.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <asm/irqflags.h>

static int irqflags = ARCH_IRQ_DISABLED;

// interrupts are TBD
unsigned long arch_local_save_flags(void)
{
	return irqflags;
}

void arch_local_irq_restore(unsigned long flags)
{
	irqflags = flags;
}

void handle_irq(int irq)
{
	unsigned long flags;

	local_irq_save(flags);
	irq_enter();
	generic_handle_irq(irq);
	irq_exit();
	local_irq_restore(flags);
}

extern void user_init_IRQ(void);
void __init init_IRQ(void)
{
	int i;
	for (i = 0; i < NR_IRQS; i++)
		irq_set_chip_and_handler(i, &dummy_irq_chip, handle_simple_irq);
	user_init_IRQ();
}
