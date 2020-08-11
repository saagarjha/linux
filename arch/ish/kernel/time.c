#include <linux/init.h>
#include <linux/sched_clock.h>
#include <asm/delay.h>
#include <user/user.h>

// TODO make these make sense
void __const_udelay(unsigned long xloops)
{
	while (xloops--) {}
}
void calibrate_delay(void)
{
}

void __init time_init(void)
{
	sched_clock_register(host_monotonic_nanos, 64, 1000000000);
}
