/*
 * prodcons.c - Producer-consumer with POSIX condition variables
 *
 * Two threads coordinate via a shared state variable:
 * - Producer waits for state==0, produces, sets state=1
 * - Consumer waits for state==1, consumes, sets state=0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <pthread.h>
#include <sched.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;

volatile int state   = 0;
volatile int product = 0;

void *producer(void *);
void *consumer(void *);

int main()
{
	pthread_create(NULL, NULL, consumer, NULL);
	pthread_create(NULL, NULL, producer, NULL);

	sleep(20);
	printf("main exiting\n");
	return 0;
}

void *producer(void *arg)
{
	while (1) {
		pthread_mutex_lock(&mutex);
		while (state == 1)
			pthread_cond_wait(&cond, &mutex);

		printf("produced %d\n", ++product);
		state = 1;
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);
		delay(100);
	}
	return NULL;
}

void *consumer(void *arg)
{
	while (1) {
		pthread_mutex_lock(&mutex);
		while (state == 0)
			pthread_cond_wait(&cond, &mutex);

		printf("consumed %d\n", product);
		state = 0;
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);
		delay(100);
	}
	return NULL;
}
