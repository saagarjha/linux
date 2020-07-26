#ifndef __ASM_ISH_UACCESS_H
#define __ASM_ISH_UACCESS_H

/* TODO: maybe don't use asm-generic/uaccess.h? practically nothing uses it aside from um */

#define __access_ok(addr, size) ((void) (addr), (void) (size), 1) // TODO

extern unsigned long raw_copy_from_user(void *to, const void __user *from, unsigned long n);
extern unsigned long raw_copy_to_user(void __user *to, const void *from, unsigned long n);
extern unsigned long raw_copy_in_user(void __user *to, const void __user *from, unsigned long n);
extern long __strncpy_from_user(char *dst, const char __user *src, long count);
extern long __strnlen_user(const void __user *str, long len);
extern unsigned long __clear_user(void __user *mem, unsigned long len);

/* Teach asm-generic/uaccess.h that we have C functions for these. */
#define __clear_user __clear_user
#define __strnlen_user __strnlen_user
#define __strncpy_from_user __strncpy_from_user

#include <asm-generic/uaccess.h>

#endif
