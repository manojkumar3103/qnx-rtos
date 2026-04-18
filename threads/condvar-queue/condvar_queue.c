/*
 * condvar_queue.c - Thread-safe queue with condvar notification
 *
 * A data provider thread adds items to a shared queue in bursts.
 * A hardware handler thread waits on a condvar, then drains the
 * queue and writes each item to "hardware" (stdout).
 */

#include <pthread.h>
#include <unistd.h>
#include <malloc.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct queueNode {
	struct queueNode *next_ptr;
	int               data;
} queueNode_t;

queueNode_t    *dataQueuep;
int             q_n_items;
int             data_ready;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;

int  add_element_to_queue(int data);
int *get_data_and_remove_from_queue(void);
void write_to_hardware(int *data);
void add_to_queue(int data);

void *hardwareHandler(void *);
void *dataProvider(void *);

int main(void)
{
	data_ready = 0;
	dataQueuep = NULL;

	if (pthread_create(NULL, NULL, hardwareHandler, NULL) != EOK) {
		fprintf(stderr, "hardwareHandler create failed\n");
		exit(EXIT_FAILURE);
	}

	if (pthread_create(NULL, NULL, dataProvider, NULL) != EOK) {
		fprintf(stderr, "dataProvider create failed\n");
		exit(EXIT_FAILURE);
	}

	sleep(20);
	return 0;
}

void *hardwareHandler(void *i)
{
	int *data;

	while (1) {
		pthread_mutex_lock(&mutex);
		while (!data_ready)
			pthread_cond_wait(&cond, &mutex);

		while ((data = get_data_and_remove_from_queue()) != NULL) {
			pthread_mutex_unlock(&mutex);
			write_to_hardware(data);
			free(data);
			pthread_mutex_lock(&mutex);
		}
		data_ready = 0;
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

void *dataProvider(void *unused)
{
	srand(getpid());
	while (1) {
		int n_loops = rand() % 10;
		for (int i = 0; i < n_loops; i++) {
			add_to_queue(n_loops * 100 + i);
			sched_yield();
		}
		int r_sleep = rand() % 15;
		delay(r_sleep * 100 + 1);
	}
	return NULL;
}

void add_to_queue(int data)
{
	pthread_mutex_lock(&mutex);
	add_element_to_queue(data);
	data_ready = 1;
	pthread_mutex_unlock(&mutex);
	pthread_cond_signal(&cond);
}

int add_element_to_queue(int data)
{
	queueNode_t *newp = malloc(sizeof(queueNode_t));
	if (newp == NULL) {
		errno = ENOMEM;
		exit(EXIT_FAILURE);
	}
	newp->next_ptr = NULL;
	newp->data = data;

	if (dataQueuep == NULL) {
		dataQueuep = newp;
	} else {
		queueNode_t *end = dataQueuep;
		while (end->next_ptr != NULL)
			end = end->next_ptr;
		end->next_ptr = newp;
	}
	q_n_items++;
	return EOK;
}

int *get_data_and_remove_from_queue(void)
{
	if (!dataQueuep)
		return NULL;

	int *theData = malloc(sizeof(int));
	if (theData == NULL) {
		errno = ENOMEM;
		exit(EXIT_FAILURE);
	}

	queueNode_t *curp = dataQueuep;
	*theData = curp->data;
	dataQueuep = dataQueuep->next_ptr;
	free(curp);
	q_n_items--;
	return theData;
}

void write_to_hardware(int *data)
{
	char buf[255];
	int ret = sprintf(buf, "Output: %d (queue: %d)\n", *data, q_n_items);
	write(STDOUT_FILENO, buf, ret);
	delay(2);
}
