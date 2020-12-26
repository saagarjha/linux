.global _ksetjmp
_ksetjmp:
	pop %rsi
	movq %rbx, 0(%rdi)
	movq %rsp, 8(%rdi)
	push %rsi
	movq %rbp, 16(%rdi)
	movq %r12, 24(%rdi)
	movq %r13, 32(%rdi)
	movq %r14, 40(%rdi)
	movq %r15, 48(%rdi)
	movq %rsi, 56(%rdi)
	xor %rax, %rax
	ret

.global _klongjmp
_klongjmp:
	movq %rsi, %rax
	/* If we're magically calling a function, let the first argument be the
	 * longjmp parameter. */
	xchg %rsi, %rdi
	movq 0(%rsi), %rbx
	movq 8(%rsi), %rsp
	movq 16(%rsi), %rbp
	movq 24(%rsi), %r12
	movq 32(%rsi), %r13
	movq 40(%rsi), %r14
	movq 48(%rsi), %r15
	jmpq *56(%rsi)
