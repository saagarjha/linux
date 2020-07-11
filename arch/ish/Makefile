KBUILD_DEFCONFIG := ish_defconfig

ifdef CONFIG_HOST_DARWIN
	KBUILD_CFLAGS += -U__weak -Wno-incompatible-library-redeclaration
	KBUILD_HOSTCPPFLAGS += -I$(srctree)arch/ish/darwin-include -I$(objtree)arch/ish/include/generated/uapi -I$(srctree)include/uapi
	KBUILD_HOSTCPPFLAGS += -D__EXPORTED_HEADERS__
	KBUILD_LDFLAGS += -platform_version macos 10.15 10.15 -arch x86_64 # TODO
endif

KBUILD_CFLAGS += -ferror-limit=0
KBUILD_CPPFLAGS += -I$(srctree)/arch/x86/include

USER_CFLAGS = -I$(srctree)/arch/ish/include
export USER_CFLAGS

core-y += arch/ish/kernel/ arch/ish/mm/
head-y += arch/ish/kernel/sections.o
