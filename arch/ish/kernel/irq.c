#include <linux/init.h>
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

extern void user_init_IRQ(void);
void __init init_IRQ(void)
{
	user_init_IRQ();
}
