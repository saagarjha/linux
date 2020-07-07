#ifndef __ASM_ISH_CURRENT_H
#define __ASM_ISH_CURRENT_H

#include <linux/thread_info.h>

extern __thread struct task_struct *current;
#define get_current() ((struct task_struct *const) current)

#endif
