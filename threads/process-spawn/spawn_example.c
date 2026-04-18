/*
 * spawn_example.c - Process creation with posix_spawn()
 *
 * Demonstrates spawning a child process, waiting for it to die
 * via SIGCHLD, and cleaning up the zombie with wait().
 */

#include <errno.h>
#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

void sigfunc(int unused) { return; }

int main(int argc, char **argv, char **envp)
{
	pid_t    pid;
	int      child_status, ret;
	sigset_t set;

	sigemptyset(&set);
	sigaddset(&set, SIGCHLD);
	pthread_sigmask(SIG_BLOCK, &set, NULL);
	signal(SIGCHLD, sigfunc);

	printf("Parent pid: %d\n", getpid());

	char *child_argv[3] = {"/system/bin/sleep", "30", NULL};
	ret = posix_spawn(&pid, "/system/bin/sleep", NULL, NULL, child_argv, envp);
	if (ret != EOK) {
		fprintf(stderr, "posix_spawn: %s\n", strerror(ret));
		exit(EXIT_FAILURE);
	}

	printf("Child pid: %d\n", pid);
	printf("Use 'pidin family' to see parent/child relationship.\n");

	if (sigwaitinfo(&set, NULL) == -1) {
		perror("sigwaitinfo");
		exit(EXIT_FAILURE);
	}

	printf("Child died (now a zombie). Cleaning up...\n");
	sleep(5);

	pid = wait(&child_status);
	if (pid == -1) {
		perror("wait");
		exit(EXIT_FAILURE);
	}
	printf("Child %d exited with status %x. Zombie cleaned.\n", pid, child_status);

	return 0;
}
