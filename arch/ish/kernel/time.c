#include <linux/init.h>
#include <asm/delay.h>

// TODO make these make sense
void __const_udelay(unsigned long xloops)
{
	while (xloops--) {}
}
void __delay(unsigned long xloops)
{
	while (xloops--) {}
}

void __init time_init(void)
{
}
