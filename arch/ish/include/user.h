#ifndef __USER_H
#define __USER_H

extern void panic(const char *fmt, ...)
	__attribute__ ((format (printf, 1, 2)));
extern int printk(const char *fmt, ...)
	__attribute__ ((format (printf, 1, 2)));

#endif
