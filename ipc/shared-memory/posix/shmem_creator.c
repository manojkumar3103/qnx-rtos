/*
 * shmem_creator.c - POSIX shared memory creator with condvar notification
 *
 * Creates a named shared memory object, initializes a process-shared
 * mutex and condvar, then updates the data every second and broadcasts
 * to waiting readers.
 *
 * Usage: shmem_creator /object_name
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "shmem_posix.h"

void unlink_and_exit(char *name)
{
	shm_unlink(name);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	int                 fd, ret;
	shmem_t            *ptr;
	pthread_mutexattr_t mutex_attr;
	pthread_condattr_t  cond_attr;

	if (argc != 2 || *argv[1] != '/') {
		printf("Usage: shmem_creator /object_name\n");
		exit(EXIT_FAILURE);
	}

	printf("Creating shared memory: '%s'\n", argv[1]);

	fd = shm_open(argv[1], O_RDWR | O_CREAT | O_EXCL, 0660);
	if (fd == -1) {
		perror("shm_open");
		unlink_and_exit(argv[1]);
	}

	ret = ftruncate(fd, sizeof(shmem_t));
	if (ret == -1) {
		perror("ftruncate");
		unlink_and_exit(argv[1]);
	}

	ptr = mmap(0, sizeof(shmem_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ptr == MAP_FAILED) {
		perror("mmap");
		unlink_and_exit(argv[1]);
	}
	close(fd);

	pthread_mutexattr_init(&mutex_attr);
	pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
	ret = pthread_mutex_init(&ptr->mutex, &mutex_attr);
	if (ret != EOK) {
		fprintf(stderr, "pthread_mutex_init: %s\n", strerror(ret));
		unlink_and_exit(argv[1]);
	}

	pthread_condattr_init(&cond_attr);
	pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);
	ret = pthread_cond_init(&ptr->cond, &cond_attr);
	if (ret != EOK) {
		fprintf(stderr, "pthread_cond_init: %s\n", strerror(ret));
		unlink_and_exit(argv[1]);
	}

	ptr->init_flag = 1;
	printf("Shared memory initialized.\n");

	while (1) {
		sleep(1);

		pthread_mutex_lock(&ptr->mutex);
		ptr->data_version++;
		snprintf(ptr->text, sizeof(ptr->text), "data update: %lu", ptr->data_version);
		pthread_mutex_unlock(&ptr->mutex);

		pthread_cond_broadcast(&ptr->cond);
	}

	munmap(ptr, sizeof(shmem_t));
	shm_unlink(argv[1]);
	return EXIT_SUCCESS;
}
