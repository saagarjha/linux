#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>

#include <user/user.h>
#define KSYM_NAME_LEN 128

const char *kallsyms_lookup(unsigned long addr, unsigned long *symbolsize, unsigned long *offset, char **modname, char *namebuf)
{
	const char *module;
	Dl_info info;
	if (dladdr((const void *) addr, &info) == 0)
		return NULL;

	if (offset)
		*offset = addr - (unsigned long) info.dli_saddr;

	module = strrchr(info.dli_fname, '/');
	if (module == NULL) {
		module = info.dli_fname;
	} else {
		module++;
	}
	if (modname)
		*modname = (char *) module;

	/* lol no one cares */
	if (symbolsize)
		*symbolsize = 0;

	strncpy(namebuf, info.dli_sname, KSYM_NAME_LEN);
	namebuf[KSYM_NAME_LEN - 1] = '\0';
	return namebuf;
}
