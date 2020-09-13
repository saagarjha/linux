#ifndef __ISH_TIME_USER_H
#define __ISH_TIME_USER_H
#include <stdint.h>

void timer_start_thread(void);
void timer_set_next_event(uint64_t ns);

#endif
