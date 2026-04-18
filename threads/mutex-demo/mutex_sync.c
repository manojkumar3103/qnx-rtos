/*
 * mutex_sync.c - Thread synchronization with mutex
 *
 * Same as nomutex.c but with proper mutex protection around
 * shared variable access, eliminating the race condition.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <pthread.h>
#include <sched.h>
#include <atomic.h>

#define NUMTHREADS 4

volatile unsigned   var1;
volatile unsigned   var2;
pthread_mutex_t     var_mutex;
volatile int        done;

void do_work(void);
void *update_thread(void *);

int main()
{
	int              ret;
	pthread_t        threadID[NUMTHREADS];
	pthread_attr_t   attrib;
	struct sched_param param;
	int              i, policy;

	var1 = var2 = 0;
	printf("mutex_sync: starting threads\n");

	ret = pthread_mutex_init(&var_mutex, NULL);
	if (ret != EOK) {
		fprintf(stderr, "pthread_mutex_init: %s\n", strerror(ret));
		exit(EXIT_FAILURE);
	}

	pthread_getschedparam(pthread_self(), &policy, &param);
	pthread_attr_init(&attrib);
	pthread_attr_setinheritsched(&attrib, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attrib, SCHED_RR);
	param.sched_priority -= 2;
	pthread_attr_setschedparam(&attrib, &param);

	for (i = 0; i < NUMTHREADS; i++) {
		ret = pthread_create(&threadID[i], &attrib, &update_thread, 0);
		if (ret != EOK) {
			fprintf(stderr, "pthread_create: %s\n", strerror(ret));
			exit(EXIT_FAILURE);
		}
	}

	sleep(15);
	done = 1;

	for (i = 0; i < NUMTHREADS; i++)
		pthread_join(threadID[i], NULL);

	printf("Done: var1=%u, var2=%u\n", var1, var2);
	return EXIT_SUCCESS;
}

void do_work(void)
{
	static volatile unsigned var3 = 1;
	if (!(atomic_add_value(&var3, 1) % 10000000))
		printf("thread %d did some work\n", pthread_self());
}

void *update_thread(void *i)
{
	int ret;
	while (!done) {
		pthread_mutex_lock(&var_mutex);
		if (var1 != var2) {
			unsigned lvar1 = var1, lvar2 = var2;
			var1 = var2;
			pthread_mutex_unlock(&var_mutex);
			printf("thread %d: var1(%u) != var2(%u)!\n", pthread_self(), lvar1, lvar2);
		} else {
			pthread_mutex_unlock(&var_mutex);
		}

		do_work();

		pthread_mutex_lock(&var_mutex);
		var1 += 2;
		var1--;
		var2 += 2;
		var2--;
		pthread_mutex_unlock(&var_mutex);
	}
	return NULL;
}
