#ifdef __linux__
#define SYM(x) x
#else
#define SYM(x) _##x
#endif

.global SYM(ksetjmp)
.align 4
SYM(ksetjmp):
	stp x19, x20, [x0, #0x0]
	stp x21, x22, [x0, #0x10]
	stp x23, x24, [x0, #0x20]
	stp x25, x26, [x0, #0x30]
	stp x27, x28, [x0, #0x40]
	stp fp, lr, [x0, #0x50]
	mov x1, sp
	str x1, [x0, #0x60]
	stp d8, d9, [x0, #0x68]
	stp d9, d11, [x0, #0x78]
	stp d10, d11, [x0, #0x88]
	stp d12, d13, [x0, #0x98]
	stp d14, d15, [x0, #0x98]
	mov x0, #0
	ret

.global SYM(klongjmp)
.align 4
SYM(klongjmp):
	ldp x19, x20, [x0, #0x0]
	ldp x21, x22, [x0, #0x10]
	ldp x23, x24, [x0, #0x20]
	ldp x25, x26, [x0, #0x30]
	ldp x27, x28, [x0, #0x40]
	ldp fp, lr, [x0, #0x50]
	ldr x2, [x0, #0x60]
	mov sp, x2
	ldp d8, d9, [x0, #0x68]
	ldp d9, d11, [x0, #0x78]
	ldp d10, d11, [x0, #0x88]
	ldp d12, d13, [x0, #0x98]
	ldp d14, d15, [x0, #0x98]
	mov x0, x1
	br lr
