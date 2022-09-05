obj-y += ia32.o ia32_signal.o vdso_vma.o
CFLAGS_vdso_vma.o += -DLIBVDSO_SO=$(obj)/libvdso.so

vobjs-y += vdso.o vdso-note.o

VDSOCC = clang
ifneq (,$(wildcard /usr/local/opt/llvm/bin/clang))
	VDSOCC = /usr/local/opt/llvm/bin/clang
endif
ifneq (,$(wildcard /opt/homebrew/opt/llvm/bin/clang))
	VDSOCC = /opt/homebrew/opt/llvm/bin/clang
endif
ifneq (,$(wildcard /opt/local/bin/clang))
	VDSOCC = /opt/local/bin/clang
endif
VDSO_CFLAGS += --target=i686-pc-linux-gnu

targets += vdso.lds $(vobjs-y)
vobjs := $(foreach F,$(vobjs-y),$(obj)/$F)
$(vobjs): KBUILD_CFLAGS = $(VDSO_CFLAGS)
$(vobjs): KBUILD_AFLAGS = $(VDSO_CFLAGS)
$(vobjs): override CC = $(VDSOCC)

quiet_cmd_vdsold = VDSOLD  $@
      cmd_vdsold = $(VDSOCC) $(VDSO_CFLAGS) -fuse-ld=lld -nostdlib \
		   -shared -Wl,-T,$(filter %.lds,$^) $(filter %.o,$^) -o $@

$(obj)/libvdso.so: $(vobjs) $(obj)/vdso.lds FORCE
	$(call if_changed,vdsold)
targets += libvdso.so

quiet_cmd_vdso_sym = VDSOSYM $@
      cmd_vdso_sym = $(NM) $< | awk '/ T __kernel/ {printf(".%s = 0x%s,\n", $$3, $$1)}' > $@
include/generated/vdso-offsets.h: $(obj)/libvdso.so FORCE
	$(call if_changed,vdso_sym)
targets += ../../../include/generated/vdso-offsets.h
