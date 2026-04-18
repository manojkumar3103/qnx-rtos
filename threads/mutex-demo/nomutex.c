/*
 * nomutex.c - Race condition demonstration (unsynchronized)
 *
 * Multiple threads increment shared variables without protection,
 * demonstrating data races when var1 and var2 diverge.
 * Compare with mutex_sync.c to see the fix.
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

volatile unsigned var1;
volatile unsigned var2;
volatile int      done;

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
	printf("nomutex: starting threads\n");

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
	static volatile unsigned var3;
	var3++;
	if (!(var3 % 10000000))
		printf("thread %d did some work\n", pthread_self());
}

void *update_thread(void *i)
{
	while (!done) {
		if (var1 != var2) {
			printf("thread %d: var1(%u) != var2(%u)!\n", pthread_self(), var1, var2);
			var1 = var2;
		}
		do_work();
		var1 += 2;
		var1--;
		var2 += 2;
		var2--;
	}
	return NULL;
}
