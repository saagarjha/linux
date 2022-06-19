#include <stdbool.h>
#if __linux__
#include <sys/random.h>
#elif __APPLE__
#include <stdlib.h>
#endif

bool arch_get_random_int(unsigned int *v) {
#if __APPLE__
	*v = arc4random();
	return true;
#elif __linux__
	int err = getrandom(v, sizeof(*v), 0);
	return err > 0;
#else
	return false;
#endif
}
