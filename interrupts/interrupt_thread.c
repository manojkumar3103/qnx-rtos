/*
 * interrupt_thread.c - Keyboard interrupt handler using IST approach
 *
 * Demonstrates InterruptAttachThread() + InterruptWait() for handling
 * hardware interrupts. The calling thread becomes the IST (Interrupt
 * Service Thread) and blocks until an interrupt fires.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/neutrino.h>

int main(int argc, char **argv)
{
	int id;
	int count = 0;

	printf("starting...\n");

	id = InterruptAttachThread(1, 0);
	if (id == -1) {
		perror("InterruptAttachThread");
		exit(EXIT_FAILURE);
	}

	while (1) {
		if (InterruptWait(_NTO_INTR_WAIT_FLAGS_FAST, NULL) == -1) {
			perror("InterruptWait");
			exit(EXIT_FAILURE);
		}

		if (InterruptUnmask(0, id) == -1)
			perror("InterruptUnmask");

		printf("Interrupt received, count = %d\n", count++);
	}
}
