/*
 * shmem_posix.h - POSIX shared memory data structure
 */

#ifndef _SHMEM_POSIX_H_
#define _SHMEM_POSIX_H_

#include <pthread.h>
#include <stdint.h>

#define MAX_TEXT_LEN 100

typedef struct {
	volatile unsigned init_flag;
	pthread_mutex_t   mutex;
	pthread_cond_t    cond;
	uint64_t          data_version;
	char              text[MAX_TEXT_LEN];
} shmem_t;

#endif
