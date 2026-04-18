/*
 * nonblockpulserec.c - Non-blocking pulse drain
 *
 * Demonstrates how to clean up a burst of queued pulses using
 * a zero-length TimerTimeout to make MsgReceivePulse() non-blocking.
 * Useful when a high-frequency interrupt source queues multiple
 * pulses between processing cycles.
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/neutrino.h>

#define OUR_PULSE_CODE (_PULSE_CODE_MINAVAIL + 7)

void *pulse_sender_thread(void *arg);
void  options(int argc, char **argv);

int verbose;
int chid;

int main(int argc, char **argv)
{
	rcvid_t       rcvid;
	int           ret;
	struct _pulse pulse;

	printf("nonblockpulserec: starting...\n");
	options(argc, argv);

	chid = ChannelCreate(_NTO_CHF_PRIVATE);
	if (chid == -1) {
		perror("ChannelCreate");
		exit(EXIT_FAILURE);
	}

	int status = pthread_create(NULL, NULL, pulse_sender_thread, NULL);
	if (status != EOK) {
		fprintf(stderr, "pthread_create: %s\n", strerror(status));
		exit(EXIT_FAILURE);
	}

	while (1) {
		rcvid = MsgReceive(chid, &pulse, sizeof(pulse), NULL);
		if (rcvid == -1) {
			perror("MsgReceive");
			exit(EXIT_FAILURE);
		}

		printf("Pulse received, draining queue");

		int flags = _NTO_TIMEOUT_RECEIVE;
		do {
			TimerTimeout(CLOCK_MONOTONIC, flags, NULL, NULL, NULL);
			ret = MsgReceivePulse(chid, &pulse, sizeof(pulse), NULL);
			if (ret == 0)
				printf(".");
		} while (ret != -1);

		printf(" done\n");
	}
}

void *pulse_sender_thread(void *arg)
{
	int coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0);
	if (coid == -1) {
		perror("ConnectAttach");
		exit(EXIT_FAILURE);
	}

	while (1) {
		sleep(5);
		for (int i = 0; i < 7; i++)
			MsgSendPulse(coid, OUR_PULSE_CODE, 0, 0);
	}
	return NULL;
}

void options(int argc, char **argv)
{
	int opt;
	verbose = 0;
	while ((opt = getopt(argc, argv, "v")) != -1) {
		if (opt == 'v')
			verbose = 1;
	}
}
