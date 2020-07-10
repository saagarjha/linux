/* SPDX-License-Identifier: GPL-2.0 */
/*
  initramfs_data includes the compressed binary that is the
  filesystem used for early user space.
  Note: Older versions of "as" (prior to binutils 2.11.90.0.23
  released on 2001-07-14) dit not support .incbin.
  If you are forced to use older binutils than that then the
  following trick can be applied to create the resulting binary:


  ld -m elf_i386  --format binary --oformat elf32-i386 -r \
  -T initramfs_data.scr initramfs_data.cpio.gz -o initramfs_data.o
   ld -m elf_i386  -r -o built-in.a initramfs_data.o

  For including the .init.ramfs sections, see include/asm-generic/vmlinux.lds.

  The above example is for i386 - the parameters vary from architectures.
  Eventually look up LDFLAGS_BLOB in an older version of the
  arch/$(ARCH)/Makefile to see the flags used before .incbin was introduced.

  Using .incbin has the advantage over ld that the correct flags are set
  in the ELF header, as required by certain architectures.
*/

#ifdef __ELF__
#define INIT_RAMFS .init.ramfs,"a"
#define SYM(x) x
#elif defined(__MACH__)
#define INIT_RAMFS __TEXT,__init_ramfs
#define SYM(x) _##x
#endif

.section INIT_RAMFS
__irf_start:
.incbin "usr/initramfs_inc_data"
__irf_end:
#ifdef __ELF__
.section .init.ramfs.info,"a"
#endif
.globl SYM(__initramfs_size)
SYM(__initramfs_size):
#ifdef CONFIG_64BIT
	.quad __irf_end - __irf_start
#else
	.long __irf_end - __irf_start
#endif
