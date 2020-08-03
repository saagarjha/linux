#ifndef __ASM_ISH_VDSO_H
#define __ASM_ISH_VDSO_H

extern struct vdso_syms {
	long __kernel_rt_sigreturn;
	long __kernel_sigreturn;
	long __kernel_vsyscall;
} vdso_syms;

#endif
