/*
 * interrupt_event.c - Keyboard interrupt handler using pulse-based events
 *
 * Demonstrates InterruptAttachEvent() to handle hardware interrupts.
 * The kernel delivers a pulse to our channel on each interrupt;
 * we process it and unmask the interrupt for the next one.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/neutrino.h>

#define INT_PULSE_CODE    (_PULSE_CODE_MINAVAIL + 0)
#define INTERRUPT_PRIORITY 15

int main(int argc, char **argv)
{
	int            id, chid, self_coid;
	int            count = 0;
	struct _pulse  msg;
	struct sigevent int_event;

	printf("starting...\n");

	chid = ChannelCreate(_NTO_CHF_PRIVATE);
	if (chid == -1) {
		perror("ChannelCreate");
		exit(EXIT_FAILURE);
	}

	self_coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0);
	if (self_coid == -1) {
		perror("ConnectAttach");
		exit(EXIT_FAILURE);
	}

	SIGEV_PULSE_INIT(&int_event, self_coid, INTERRUPT_PRIORITY, INT_PULSE_CODE, 0);

	id = InterruptAttachEvent(1, &int_event, 0);
	if (id == -1) {
		perror("InterruptAttachEvent");
		exit(EXIT_FAILURE);
	}

	while (1) {
		if (MsgReceivePulse(chid, &msg, sizeof(msg), NULL) == -1) {
			perror("MsgReceivePulse");
			exit(EXIT_FAILURE);
		}

		if (msg.code == INT_PULSE_CODE) {
			if (InterruptUnmask(0, id) == -1)
				perror("InterruptUnmask");
			printf("Interrupt received, count = %d\n", count++);
		} else {
			fprintf(stderr, "Unexpected pulse code: %d\n", msg.code);
		}
	}
}
