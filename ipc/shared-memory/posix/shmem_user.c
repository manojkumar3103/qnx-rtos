/*
 * shmem_user.c - POSIX shared memory reader with condvar wait
 *
 * Opens an existing shared memory object, waits for data changes using
 * condvar, and prints updates as they arrive.
 *
 * Usage: shmem_user /object_name
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

void *get_shared_memory_pointer(char *name, unsigned num_retries)
{
	unsigned  tries;
	shmem_t  *ptr;
	int       fd;

	for (tries = 0; ; ) {
		fd = shm_open(name, O_RDWR, 0);
		if (fd != -1) break;
		if (++tries > num_retries) {
			perror("shm_open");
			return MAP_FAILED;
		}
		sleep(1);
	}

	for (tries = 0; ; ) {
		ptr = mmap(0, sizeof(shmem_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (ptr != MAP_FAILED) break;
		if (++tries > num_retries) {
			perror("mmap");
			return MAP_FAILED;
		}
		sleep(1);
	}
	close(fd);

	for (tries = 0; ; ) {
		if (ptr->init_flag) break;
		if (++tries > num_retries) {
			fprintf(stderr, "init flag never set\n");
			munmap(ptr, sizeof(shmem_t));
			return MAP_FAILED;
		}
		sleep(1);
	}

	return ptr;
}

int main(int argc, char *argv[])
{
	int      ret;
	shmem_t *ptr;
	uint64_t last_version = 0;
	char     local_copy[MAX_TEXT_LEN];

	if (argc != 2 || *argv[1] != '/') {
		printf("Usage: shmem_user /object_name\n");
		exit(EXIT_FAILURE);
	}

	ptr = get_shared_memory_pointer(argv[1], 100);
	if (ptr == MAP_FAILED) {
		fprintf(stderr, "Unable to access '%s'\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	while (1) {
		ret = pthread_mutex_lock(&ptr->mutex);
		if (ret != EOK) {
			fprintf(stderr, "pthread_mutex_lock: %s\n", strerror(ret));
			exit(EXIT_FAILURE);
		}

		while (last_version == ptr->data_version) {
			ret = pthread_cond_wait(&ptr->cond, &ptr->mutex);
			if (ret != EOK) {
				fprintf(stderr, "pthread_cond_wait: %s\n", strerror(ret));
				exit(EXIT_FAILURE);
			}
		}

		last_version = ptr->data_version;
		strlcpy(local_copy, ptr->text, sizeof(local_copy));

		pthread_mutex_unlock(&ptr->mutex);

		printf("Data: '%s'\n", local_copy);
	}

	return EXIT_SUCCESS;
}
