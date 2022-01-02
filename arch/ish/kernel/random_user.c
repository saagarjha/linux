#include <stdlib.h>
#include <stdbool.h>

bool arch_get_random_int(unsigned int *v) {
#if __APPLE__
	*v = arc4random();
	return true;
#else
	return false;
#endif
}
