#include <linux/reboot.h>

void (*pm_power_off)(void);
EXPORT_SYMBOL(pm_power_off);

extern void exit(int) __noreturn;

void machine_halt(void)
{
	exit(0);
}
void machine_power_off(void)
{
	machine_halt();
}
void machine_restart(char *cmd)
{
	machine_halt();
}

void machine_emergency_restart(void)
{
#if defined(__clang__)
	__builtin_debugtrap();
#else
	__builtin_trap();
#endif
}
