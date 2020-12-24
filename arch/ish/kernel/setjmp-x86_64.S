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
	movq 0(%rdi), %rbx
	movq 8(%rdi), %rsp
	movq 16(%rdi), %rbp
	movq 24(%rdi), %r12
	movq 32(%rdi), %r13
	movq 40(%rdi), %r14
	movq 48(%rdi), %r15
	jmpq *56(%rdi)
