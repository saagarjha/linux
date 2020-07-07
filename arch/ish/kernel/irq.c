#include <linux/init.h>
#include <asm/irqflags.h>

// interrupts are TBD
unsigned long arch_local_save_flags(void)
{
	return 0;
}

void arch_local_irq_restore(unsigned long flags)
{
	(void) flags;
}

void __init init_IRQ(void)
{
}
