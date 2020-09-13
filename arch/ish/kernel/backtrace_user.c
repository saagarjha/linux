#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>

#include <user/user.h>

int lookup_symbol(void *addr, struct sym_info *out)
{
	Dl_info info;
	if (dladdr(addr, &info) == 0)
		return 0;
	out->symbol = info.dli_sname;
	out->symbol_addr = info.dli_saddr;

	out->module = strrchr(info.dli_fname, '/');
	if (out->module == NULL) {
		out->module = info.dli_fname;
	} else {
		out->module++;
	}
	return 1;
}
