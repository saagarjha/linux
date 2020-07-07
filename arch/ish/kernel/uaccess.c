#include <linux/uaccess.h>

unsigned long raw_copy_from_user(void *to, const void __user *from, unsigned long n)
{
	__builtin_trap();
}
unsigned long raw_copy_to_user(void __user *to, const void *from, unsigned long n)
{
	__builtin_trap();
}
long __strncpy_from_user(char *dst, const char __user *src, long count)
{
	__builtin_trap();
}
long __strnlen_user(const void __user *str, long len)
{
	__builtin_trap();
}
unsigned long __clear_user(void __user *mem, unsigned long len)
{
	__builtin_trap();
}
