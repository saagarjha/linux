#define __NR_time 13
#define __NR_gettimeofday 78
#define __NR_sigreturn 119
#define __NR_rt_sigreturn 173
#define __NR_clock_gettime 265

void __attribute__((naked)) __kernel_vsyscall()
{
	asm volatile("int $0x80; ret");
}
void __attribute__((naked)) __kernel_sigreturn()
{
	asm volatile("popl %%eax; movl %0, %%eax; int $0x80" :: "i" (__NR_sigreturn));
}
void __attribute__((naked)) __kernel_rt_sigreturn()
{
	asm volatile("movl %0, %%eax; int $0x80" :: "i" (__NR_rt_sigreturn));
}

int __vdso_clock_gettime(long clock, void *ts)
{
	long ret;
	asm("int $0x80" : "=a" (ret) :
		"0" (__NR_clock_gettime), "D" (clock), "S" (ts) : "memory");
	return ret;
}
int clock_gettime(long, void *)
	__attribute__((weak, alias("__vdso_clock_gettime")));

int __vdso_gettimeofday(void *tv, void *tz)
{
	long ret;
	asm("int $0x80" : "=a" (ret) :
		"0" (__NR_gettimeofday), "D" (tv), "S" (tz) : "memory");
	return ret;
}
int gettimeofday(void *, void *)
	__attribute__((weak, alias("__vdso_gettimeofday")));

long __vdso_time(long *t)
{
	long secs;
	asm volatile("int $0x80"
		: "=a" (secs)
		: "0" (__NR_time), "D" (t) : "cc", "r11", "cx", "memory");
	return secs;
}
long time(long *t) __attribute__((weak, alias("__vdso_time")));

long
__vdso_getcpu(unsigned *cpu, unsigned *node, void *unused)
{
	if (cpu)
		*cpu = 0;
	if (node)
		*node = 0;
	return 0;
}
long getcpu(unsigned *cpu, unsigned *node, void *tcache)
	__attribute__((weak, alias("__vdso_getcpu")));
