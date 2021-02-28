ifndef EMULATOR
	EMULATOR := ishemu
endif
export EMULATOR

KBUILD_DEFCONFIG := ish_defconfig

KBUILD_CFLAGS += -ffreestanding -fPIE

ifdef CONFIG_HOST_DARWIN
	KBUILD_CFLAGS += -U__weak -Wno-incompatible-library-redeclaration
	KBUILD_HOSTCPPFLAGS += -I$(srctree)arch/ish/darwin-include -I$(objtree)arch/ish/include/generated/uapi -I$(srctree)include/uapi
	KBUILD_HOSTCPPFLAGS += -D__EXPORTED_HEADERS__
	KBUILD_LDFLAGS += -platform_version macos 10.15 10.15 -arch x86_64 # TODO
endif

USER_CFLAGS = -I$(srctree)/arch/ish/include
export USER_CFLAGS

core-y += arch/ish/kernel/ arch/ish/mm/ arch/ish/drivers/ arch/ish/net/
ifdef CONFIG_HOST_DARWIN
	head-y += arch/ish/kernel/sections.o
endif

core-y += arch/ish/$(EMULATOR)/
KBUILD_CPPFLAGS += -I$(srctree)/arch/ish/include/$(EMULATOR)
ifeq ($(EMULATOR),ishemu)
	KBUILD_CPPFLAGS += -I../..
endif

include arch/ish/Makefile.abi

export CFLAGS_vmlinux
ifdef CONFIG_HOST_DARWIN
	CFLAGS_vmlinux += -lpthread
	CFLAGS_vmlinux += -L../../build -lish_emu
	CFLAGS_vmlinux += -sectalign __DATA __percpu_first 1000
	CFLAGS_vmlinux += -sectalign __DATA __tracepoints 20
endif
# TODO: flags on linux host
vmlinux: ../../build/libish_emu.a

# TODO: $(SUBARCH) doesn't work because it's x86_64
UTS_MACHINE := i686
