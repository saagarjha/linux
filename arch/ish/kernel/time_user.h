#ifndef __ISH_TIME_USER_H
#define __ISH_TIME_USER_H
#ifndef __KERNEL__
#include <stdint.h>
#endif

void timer_start_thread(void);
void timer_set_next_event(uint64_t ns);

#endif
