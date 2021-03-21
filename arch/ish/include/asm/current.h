#ifndef __ASM_ISH_CURRENT_H
#define __ASM_ISH_CURRENT_H

extern __thread struct task_struct *__current;
#define get_current() ((struct task_struct *const) __current)
#define current get_current()
#define in_kernel() (current != NULL)

#endif
