// TODO: Putting this arch kernel include directory on the user code include
// path is a layering violation which causes fuckups when system headers
// include asm/unistd.h and this takes precedence over the system version.
// Here's a quick hack for now, but we should really get this out of the
// include path. Putting it off since it requires a serious header organization
// (adding a special directory for headers shared between user and kernel code,
// and migrating a lot of code over there.)
#if !defined(__KERNEL__)
#include_next <asm/unistd.h>
#else

#ifndef __ASM_ISH_UNISTD_H
#define __ASM_ISH_UNISTD_H

/* for x86 */
#include <asm/unistd_32.h>

#define NR_syscalls __NR_syscalls

/* ugh */
# define __ARCH_WANT_NEW_STAT
# define __ARCH_WANT_OLD_READDIR
# define __ARCH_WANT_OLD_STAT
# define __ARCH_WANT_SYS_ALARM
# define __ARCH_WANT_SYS_FADVISE64
# define __ARCH_WANT_SYS_GETHOSTNAME
# define __ARCH_WANT_SYS_GETPGRP
# define __ARCH_WANT_SYS_NICE
# define __ARCH_WANT_SYS_OLDUMOUNT
# define __ARCH_WANT_SYS_OLD_GETRLIMIT
# define __ARCH_WANT_SYS_OLD_UNAME
# define __ARCH_WANT_SYS_PAUSE
# define __ARCH_WANT_SYS_SIGNAL
# define __ARCH_WANT_SYS_SIGPENDING
# define __ARCH_WANT_SYS_SIGPROCMASK
# define __ARCH_WANT_SYS_SOCKETCALL
# define __ARCH_WANT_SYS_TIME32
# define __ARCH_WANT_SYS_UTIME32
# define __ARCH_WANT_SYS_WAITPID
# define __ARCH_WANT_SYS_FORK
# define __ARCH_WANT_SYS_VFORK
# define __ARCH_WANT_SYS_CLONE
# define __ARCH_WANT_SYS_CLONE3

#  define __ARCH_WANT_SYS_TIME
#  define __ARCH_WANT_SYS_UTIME
#  define __ARCH_WANT_COMPAT_SYS_PREADV64
#  define __ARCH_WANT_COMPAT_SYS_PWRITEV64
#  define __ARCH_WANT_COMPAT_SYS_PREADV64V2
#  define __ARCH_WANT_COMPAT_SYS_PWRITEV64V2

#endif
#endif
