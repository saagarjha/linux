#ifndef __USER_SETJMP_H
#define __USER_SETJMP_H

// inspired by um

typedef struct {
#ifdef __x86_64__
	unsigned long rbx, rsp, rbp, r12, r13, r14, r15, rip;
#define jb_ip rip
#define jb_sp rsp
#define jb_fp rbp
#elif __aarch64__
	unsigned long r19, r20, r21, r22, r23, r24, r25, r26, r27, r28, fp, lr, sp;
	double v8, v9, v10, v11, v12, v13, v14, v15;
#define jb_ip lr
#define jb_sp sp
#define jb_fp fp
#else
#error "need to implement setjmp for your architecture"
#endif
} kjmp_buf[1];

unsigned long ksetjmp(kjmp_buf);
void klongjmp(kjmp_buf, unsigned long);

#endif
