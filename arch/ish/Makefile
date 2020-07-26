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

KBUILD_CPPFLAGS += -I$(srctree)/arch/ish/include/abi

KBUILD_CFLAGS += -ferror-limit=0

# TODO: this bugs me. we really only want <asm/user.h> but it can result in
# other things getting in without anyone knowing.
#KBUILD_CPPFLAGS += -I$(srctree)/arch/x86/include

USER_CFLAGS = -I$(srctree)/arch/ish/include
export USER_CFLAGS

core-y += arch/ish/kernel/ arch/ish/mm/ arch/ish/abi/
head-y += arch/ish/kernel/sections.o

core-y += arch/ish/$(EMULATOR)/
KBUILD_CPPFLAGS += -I$(srctree)/arch/ish/include/$(EMULATOR)
ifeq ($(EMULATOR),ishemu)
	KBUILD_CPPFLAGS += -I../..
endif

export CFLAGS_vmlinux := -lish_emu -L../../build
vmlinux: ../../build/libish_emu.a

archheaders:
	$(Q)$(MAKE) $(build)=arch/ish/syscalls all
