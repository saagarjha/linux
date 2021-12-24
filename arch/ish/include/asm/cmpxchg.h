#ifndef __ISH_ASM_CMPXCHG_H
#define __ISH_ASM_CMPXCHG_H

#include <linux/build_bug.h>

#if defined(__x86_64__)
#define __xchg_asm(ptr, t, s, gs, ret)                                 \
	asm volatile("xchg" #s " %" #gs "0, %1\n"                          \
		     : "+r"(ret), "+m"(*(volatile t *)(ptr))                   \
		     :                                                         \
		     : "memory", "cc")
#define __cmpxchg_asm(ptr, t, s, gs, ret, old, new)                    \
	asm volatile("lock cmpxchg" #s " %"#gs"2,%1"                       \
		     : "=a"(ret), "+m"(*(volatile t *)ptr)                     \
		     : "r"(new), "0"(old)                                      \
		     : "memory")

#define __xchg_op1(op, ptr, ...) __##op##_asm(ptr, u8, b, b, ##__VA_ARGS__)
#define __xchg_op2(op, ptr, ...) __##op##_asm(ptr, u16, w, w, ##__VA_ARGS__)
#define __xchg_op4(op, ptr, ...) __##op##_asm(ptr, u32, l, k, ##__VA_ARGS__)
#define __xchg_op8(op, ptr, ...) __##op##_asm(ptr, u64, q, q, ##__VA_ARGS__)

#elif defined(__aarch64__)
// TODO: Use ARMv8.1 atomics

// See this commit message for why ldxr is paired with stlxr:
// https://github.com/torvalds/linux/commit/8e86f0b409a44193f1587e87b69c5dcf8f65be67
#define __xchg_asm(ptr, t, s, gs, ret)                                 \
	__typeof__(ret) value = ret;                                       \
	__typeof__(ret) temp;                                              \
	asm volatile("1:\n"                                                \
		"ldxr" #s " %" #gs "0, [%2]\n"                                 \
		"stlxr" #s " %w1, %" #gs "3, [%2]\n"                           \
		"cbnz %w1, 1b\n"                                               \
		: "=&r"(ret), "=&r"(temp)                                      \
		: "r"(ptr), "r"(value)                                         \
		: "memory", "cc")

#define __cmpxchg_asm(ptr, t, s, gs, ret, old, new)                    \
	__typeof__(ret) temp;                                              \
	asm volatile("1:\n"                                                \
		"mov %w1, #0\n"                                                \
		"ldxr" #s " %" #gs "0, [%2]\n"                                 \
		"cmp %" #gs "0, %" #gs "3\n"                                   \
		"bne 2f\n"                                                     \
		"stxr" #s " %w1, %" #gs "4, [%2]\n"                            \
		"2:"                                                           \
		"cbnz %w1, 1b\n"                                               \
		: "=&r"(ret), "=&r"(temp)                                      \
		: "r"(ptr), "r"(old), "r"(new)                                 \
		: "memory", "cc")

#define __xchg_op1(op, ptr, ...) __##op##_asm(ptr, u8, b, w, ##__VA_ARGS__)
#define __xchg_op2(op, ptr, ...) __##op##_asm(ptr, u16, h, w, ##__VA_ARGS__)
#define __xchg_op4(op, ptr, ...) __##op##_asm(ptr, u32, , w, ##__VA_ARGS__)
#define __xchg_op8(op, ptr, ...) __##op##_asm(ptr, u64, , x, ##__VA_ARGS__)
#else
#error unsupported architecture
#endif

#define __size_dispatch(ptr, macro, ...)                               \
	({                                                                 \
		int size = sizeof(*(ptr));                                     \
		if (size == 1) {                                               \
			macro##1(__VA_ARGS__);                                     \
		} else if (size == 2) {                                        \
			macro##2(__VA_ARGS__);                                     \
		} else if (size == 4) {                                        \
			macro##4(__VA_ARGS__);                                     \
		} else if (size == 8) {                                        \
			macro##8(__VA_ARGS__);                                     \
		} else {                                                       \
			BUILD_BUG();                                               \
			unreachable();                                             \
		}                                                              \
	})

#define xchg(ptr, v)                                                   \
	({                                                                 \
		__typeof__(*(ptr)) __ret = (v);                                \
		__size_dispatch((ptr), __xchg_op, xchg, (ptr), __ret);         \
		__ret;                                                         \
	})
#define cmpxchg(ptr, old, new)                                         \
	({                                                                 \
		__typeof__(*(ptr)) __ret;                                      \
		__size_dispatch((ptr), __xchg_op, cmpxchg, (ptr), __ret, old,  \
				new);                                                  \
		__ret;                                                         \
	})

#endif
