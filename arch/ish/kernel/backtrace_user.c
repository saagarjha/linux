#include <execinfo.h>
#include <stdlib.h>

void walk_backtrace(void (*cb)(void *, unsigned long, char *), void *data)
{
	void *stack[128];
	int stack_size = backtrace(stack, 128);
	char **stack_syms = backtrace_symbols(stack, stack_size);
	int i;
	for (i = 0; i < stack_size; i++) {
		cb(data, (unsigned long) stack[i], stack_syms[i]);
	}
	free(stack_syms);
}
