/*
 * reptimer.c - Repeating timer with pulse delivery
 *
 * Creates a POSIX timer that fires a pulse after 5 seconds,
 * then every 1.5 seconds thereafter. Demonstrates timer_create()
 * with CLOCK_MONOTONIC and pulse-based notification.
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>

#define TIMER_PULSE_EVENT (_PULSE_CODE_MINAVAIL + 7)

typedef union {
	struct _pulse pulse;
} message_t;

int main(int argc, char *argv[])
{
	rcvid_t         rcvid;
	struct sigevent event;
	int             chid, coid;
	message_t       msg;
	timer_t         timerid;
	struct itimerspec it;

	chid = ChannelCreate(_NTO_CHF_PRIVATE);
	if (chid == -1) {
		perror("ChannelCreate");
		exit(EXIT_FAILURE);
	}

	coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0);
	if (coid == -1) {
		perror("ConnectAttach");
		exit(EXIT_FAILURE);
	}

	SIGEV_PULSE_INIT(&event, coid, 10, TIMER_PULSE_EVENT, 0);

	if (timer_create(CLOCK_MONOTONIC, &event, &timerid) == -1) {
		perror("timer_create");
		exit(EXIT_FAILURE);
	}

	it.it_value.tv_sec     = 5;
	it.it_value.tv_nsec    = 0;
	it.it_interval.tv_sec  = 1;
	it.it_interval.tv_nsec = 500 * 1000 * 1000;

	if (timer_settime(timerid, 0, &it, NULL) == -1) {
		perror("timer_settime");
		exit(EXIT_FAILURE);
	}

	printf("Timer armed: first in 5s, then every 1.5s\n");

	while (1) {
		rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
		if (rcvid == -1) {
			perror("MsgReceive");
			continue;
		}

		if (rcvid == 0 && msg.pulse.code == TIMER_PULSE_EVENT)
			printf("Timer expired\n");
		else
			printf("Unexpected pulse code: %d\n", msg.pulse.code);
	}
}
