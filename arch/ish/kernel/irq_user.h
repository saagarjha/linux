#ifndef __IRQ_USER_H
#define __IRQ_USER_H

void user_init_IRQ(void);
void user_setup_thread(void);

int check_irqs(void);
void trigger_irq_check(int cpu);
/* just for the idle loop */
int enable_and_check_irqs(void);

int get_smp_processor_id(void);

#endif
