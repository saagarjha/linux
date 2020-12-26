#ifndef __IRQ_USER_H
#define __IRQ_USER_H

void user_init_IRQ(void);
void user_set_irqs_enabled(int enabled);

void handle_irq(int irq);

#endif
