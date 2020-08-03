extra-y += sections.o

obj-y := \
	cpu.o \
	irq.o \
	mem.o \
	reboot.o \
	setup.o \
	signal.o \
	time.o \
	trap.o \
	process.o \
	ptrace.o \
	uaccess.o \
	syscalls.o \

obj-y += backtrace_user.o syscalls_user.o
obj-y += setjmp-$(shell uname -m).o # TODO this is disgusting and won't make sense if you want to cross compile

obj-$(CONFIG_EARLY_PRINTK) += early_printk.o

include arch/um/scripts/Makefile.rules
