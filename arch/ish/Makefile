ifndef EMULATOR
	EMULATOR := ishemu
endif
export EMULATOR

KBUILD_DEFCONFIG := ish_defconfig

ifdef CONFIG_HOST_DARWIN
	KBUILD_CFLAGS += -U__weak -Wno-incompatible-library-redeclaration
	KBUILD_HOSTCPPFLAGS += -I$(srctree)arch/ish/darwin-include -I$(objtree)arch/ish/include/generated/uapi -I$(srctree)include/uapi
	KBUILD_HOSTCPPFLAGS += -D__EXPORTED_HEADERS__
	KBUILD_LDFLAGS += -platform_version macos 10.15 10.15 -arch x86_64 # TODO
endif

USER_CFLAGS = -I$(srctree)/arch/ish/include
export USER_CFLAGS

core-y += arch/ish/kernel/ arch/ish/mm/ arch/ish/drivers/ arch/ish/net/
head-y += arch/ish/kernel/sections.o

core-y += arch/ish/$(EMULATOR)/
KBUILD_CPPFLAGS += -I$(srctree)/arch/ish/include/$(EMULATOR)
ifeq ($(EMULATOR),ishemu)
	KBUILD_CPPFLAGS += -I../..
endif

include arch/ish/Makefile.abi

export CFLAGS_vmlinux := -lish_emu -L../../build
vmlinux: ../../build/libish_emu.a

# TODO: $(SUBARCH) doesn't work because it's x86_64
UTS_MACHINE := i686
