#include <linux/sched/signal.h>
#include <asm/pgtable.h>
#include <stdarg.h>

#include "emu/interrupt.h"
#include "emu/cpu.h"

/* TODO: put this in a header */
extern int handle_page_fault(unsigned long address, int is_write, int *code_out);

extern pte_t *virt_to_pte(struct mm_struct *mm, unsigned long addr);

void handle_interrupt(int interrupt, unsigned long fault_addr, int is_write)
{
	struct ksignal ksig;

	if (interrupt == INT_GPF) {
		int code;
		int err = handle_page_fault(fault_addr, is_write, &code);
		if (err != 0)
			force_sig_fault(SIGSEGV, code, (void __user *) fault_addr);
	} else if (interrupt == INT_UNDEFINED) {
		force_sig_fault(SIGILL, SI_KERNEL, (void __user *) fault_addr);
	} else if (interrupt != INT_SYSCALL && interrupt != INT_TIMER) {
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

// linker error fixes
int current_pid(void)
{
	return current->pid;
}
void die(const char *message)
{
	panic(message);
}
void ish_printk(const char *msg, ...)
{
	static __thread char log_line[256] = "";
	va_list args;
	va_start(args, msg);
	vsnprintf(log_line + strlen(log_line), sizeof(log_line) - strlen(log_line), msg, args);
	va_end(args);
	if (msg[strlen(msg) - 1] == '\n') {
		printk("%s", log_line);
		log_line[0] = '\0';
	}
}
