#include <linux/sched/signal.h>
#include "../../../../emu/interrupt.h"

#include "../../../../emu/cpu.h"

/* TODO: put this in a header */
extern int handle_page_fault(unsigned long address, int is_write, int *code_out);

void handle_interrupt(int interrupt, unsigned long fault_addr)
{
	struct ksignal ksig;

	if (interrupt == INT_GPF) {
		int code;
		int err = handle_page_fault(fault_addr, 0 /* TODO */, &code);
		if (err != 0)
			force_sig_fault(SIGSEGV, code, (void __user *) fault_addr);
	} else if (interrupt == INT_UNDEFINED) {
		force_sig_fault(SIGILL, SI_KERNEL, (void __user *) fault_addr);
	} else if (interrupt != INT_SYSCALL) {
		force_sig_fault(SIGSEGV, SI_KERNEL, 0);
	}
	
	if (get_signal(&ksig)) {
		panic("the risk I took was calculated, but man, am I bad at math.");
	}
	/* TODO signal restarting */
	restore_saved_sigmask();
}

// this is stupid
void __user *arch_compat_alloc_user_space(long len)
{
	return NULL;
}
