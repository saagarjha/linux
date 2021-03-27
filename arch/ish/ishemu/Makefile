ccflags-y += $(ISH_CFLAGS)
USER_CFLAGS += $(ISH_CFLAGS) -I$(srctree)/arch/ish/include/$(EMULATOR)

obj-y = exec_user.o exec.o

include arch/um/scripts/Makefile.rules
