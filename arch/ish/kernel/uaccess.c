#include <linux/uaccess.h>

unsigned long raw_copy_from_user(void *to, const void __user *from, unsigned long n)
{
	if (uaccess_kernel()) {
		memcpy(to, (__force const void *) from, n);
		return 0;
	}
	__builtin_trap();
}
unsigned long raw_copy_to_user(void __user *to, const void *from, unsigned long n)
{
	if (uaccess_kernel()) {
		memcpy((__force void *) to, from, n);
		return 0;
	}
	__builtin_trap();
}
long __strncpy_from_user(char *dst, const char __user *src, long count)
{
	if (uaccess_kernel()) {
		strncpy(dst, (__force const char *) src, count);
		return strnlen(dst, count);
	}
	__builtin_trap();
}
long __strnlen_user(const void __user *str, long len)
{
	if (uaccess_kernel()) {
		return strnlen((__force const char *) str, len) + 1;
	}
	__builtin_trap();
}
unsigned long __clear_user(void __user *mem, unsigned long len)
{
	if (uaccess_kernel()) {
		memset((__force void *) mem, 0, len);
		return 0;
	}
	__builtin_trap();
}
