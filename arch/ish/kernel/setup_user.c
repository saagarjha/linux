#include <pthread.h>
#include <stdbool.h>

static pthread_mutex_t status_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t status_cond = PTHREAD_COND_INITIALIZER;
static bool started = false;

int notify_startup(void) {
	pthread_mutex_lock(&status_lock);
	started = true;
	pthread_cond_broadcast(&status_cond);
	pthread_mutex_unlock(&status_lock);
	return 0;
}

void wait_for_startup(void) {
	pthread_mutex_lock(&status_lock);
	while (!started)
		pthread_cond_wait(&status_cond, &status_lock);
	pthread_mutex_unlock(&status_lock);
}
