obj-y += ia32.o ia32_signal.o

obj-y += vdso_vma.o
$(obj)/vdso_vma.o: $(obj)/libvdso.so $(obj)/vdso_sym.h
CFLAGS_vdso_vma.o += -DLIBVDSO_SO=$(obj)/libvdso.so -DVDSO_SYM_H=../../../$(obj)/vdso_sym.h

vobjs-y += vdso.o vdso-note.o

targets += vdso.lds $(vobjs-y)
vobjs := $(foreach F,$(vobjs-y),$(obj)/$F)
$(vobjs): KBUILD_CFLAGS += --target=i686-linux-gnu
$(vobjs): KBUILD_AFLAGS += --target=i686-linux-gnu

VDSOCC = clang
ifneq (,$(wildcard /usr/local/opt/llvm/bin/clang))
	VDSOCC = /usr/local/opt/llvm/bin/clang
endif
quiet_cmd_vdsocc = VDSOLD  $@
      cmd_vdsocc = $(VDSOCC) --target=i686-linux-gnu -fuse-ld=lld -nostdlib \
		   -shared -Wl,-T,$(filter %.lds,$^) $(filter %.o,$^) -o $@

$(obj)/libvdso.so: $(vobjs) $(obj)/vdso.lds
	$(call if_changed,vdsocc)

quiet_cmd_vdso_sym = VDSOSYM $@
      cmd_vdso_sym = nm $< | awk '/ T __kernel/ {printf(".%s = 0x%s,\n", $$3, $$1)}' > $@
$(obj)/vdso_sym.h: $(obj)/libvdso.so
	$(call if_changed,vdso_sym)
