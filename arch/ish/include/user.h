#ifndef __USER_H
#define __USER_H

extern void panic(const char *fmt, ...)
	__attribute__ ((format (printf, 1, 2)));
extern int _printk(const char *fmt, ...)
	__attribute__ ((format (printf, 1, 2)));
#define printk(...) _printk(__VA_ARGS__)

#endif
