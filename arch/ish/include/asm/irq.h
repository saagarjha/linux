#ifndef __ASM_ISH_IRQ_H
#define __ASM_ISH_IRQ_H

/* TODO: allocate dynamically */

#define TIMER_IRQ 0
#define STDIN_IRQ 1
#define HOST_INET_IRQ 3
#define NR_IRQS 16

void *current_irq_data(void);

#endif
