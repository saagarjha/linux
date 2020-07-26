#ifndef __USER_SETJMP_H
#define __USER_SETJMP_H

// inspired by um

typedef struct {
#ifdef __x86_64__
	unsigned long rbx, rsp, rbp, r12, r13, r14, r15, rip;
#define jb_ip rip
#define jb_sp rsp
#else
#error "need to implement setjmp for your architecture"
#endif
} kjmp_buf[1];

unsigned long ksetjmp(kjmp_buf);
void klongjmp(kjmp_buf, unsigned long);

#endif
