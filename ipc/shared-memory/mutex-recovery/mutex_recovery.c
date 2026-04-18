/*
 * mutex_recovery.c - Robust mutex recovery after process death
 *
 * Demonstrates PTHREAD_MUTEX_ROBUST with process-shared mutexes.
 * A child process locks the mutex and dies; the parent detects
 * EOWNERDEAD and recovers via pthread_mutex_consistent().
 */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <sys/wait.h>

typedef struct {
	pthread_mutex_t mtx;
} shmem_t;

int main(int argc, char *argv[])
{
	int ret;

	int fd = shm_open(SHM_ANON, O_RDWR | O_CREAT, 0600);
	if (fd == -1) {
		perror("shm_open");
		exit(EXIT_FAILURE);
	}

	if (ftruncate(fd, 4096) == -1) {
		perror("ftruncate");
		exit(EXIT_FAILURE);
	}

	shmem_t *ptr = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ptr == MAP_FAILED) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}
	close(fd);

	pthread_mutexattr_t mattr;
	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_setrobust(&mattr, PTHREAD_MUTEX_ROBUST);
	pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);

	ret = pthread_mutex_init(&ptr->mtx, &mattr);
	if (ret != EOK) {
		fprintf(stderr, "pthread_mutex_init: %s\n", strerror(ret));
		exit(EXIT_FAILURE);
	}

	pid_t pid = fork();
	if (pid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if (pid == 0) {
		ret = pthread_mutex_lock(&ptr->mtx);
		if (ret != EOK) {
			fprintf(stderr, "child pthread_mutex_lock: %s\n", strerror(ret));
			exit(EXIT_FAILURE);
		}
		printf("Child exiting with mutex locked\n");
		exit(EXIT_SUCCESS);
	}

	int tstatus;
	pid = waitpid(pid, &tstatus, 0);
	if (pid == -1) {
		perror("waitpid");
		exit(EXIT_FAILURE);
	}

	if (WIFEXITED(tstatus) && WEXITSTATUS(tstatus) == EXIT_SUCCESS) {
		ret = pthread_mutex_lock(&ptr->mtx);
		if (ret == EOWNERDEAD) {
			ret = pthread_mutex_consistent(&ptr->mtx);
			if (ret != EOK) {
				fprintf(stderr, "pthread_mutex_consistent: %s\n", strerror(ret));
				exit(EXIT_FAILURE);
			}
			printf("Parent recovered the mutex\n");

			ret = pthread_mutex_unlock(&ptr->mtx);
			if (ret != EOK) {
				fprintf(stderr, "pthread_mutex_unlock: %s\n", strerror(ret));
				exit(EXIT_FAILURE);
			}
		}
	}

	return 0;
}
