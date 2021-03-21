#ifndef __ISH_USER_IRQ_H
#define __ISH_USER_IRQ_H

#include <asm/irq.h>

void trigger_irq(int irq);
void trigger_irq_on_cpu(int irq, int cpu);

#endif
