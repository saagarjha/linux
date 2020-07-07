#include <linux/reboot.h>

void (*pm_power_off)(void);
EXPORT_SYMBOL(pm_power_off);

void machine_halt(void)
{
}
void machine_power_off(void)
{
	machine_halt();
}
void machine_restart(char *cmd)
{
	machine_halt();
}
