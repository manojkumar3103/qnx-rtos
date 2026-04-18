/*
 * death_pulse.c - Process death notification via system manager
 *
 * Requests PROCMGR_EVENT_PROCESS_DEATH notifications from the QNX
 * process manager. Prints the PID of every process that dies.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/procmgr.h>
#include <process.h>

int main(void)
{
	int            chid, coid, ret;
	struct _pulse  pulse;
	struct sigevent ev;

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

	SIGEV_PULSE_INIT(&ev, coid, 10, 1, 0);
	SIGEV_MAKE_UPDATEABLE(&ev);

	MsgRegisterEvent(&ev, SYSMGR_COID);
	procmgr_event_notify(PROCMGR_EVENT_PROCESS_DEATH, &ev);

	printf("Waiting for process death notifications...\n");

	while (1) {
		ret = MsgReceivePulse(chid, &pulse, sizeof(pulse), NULL);
		if (ret == -1) {
			perror("MsgReceivePulse");
			exit(EXIT_FAILURE);
		}

		if (pulse.code == 1)
			printf("Process died: pid %d\n", pulse.value.sival_int);
		else
			printf("Unexpected pulse code: %d\n", pulse.code);
	}
	return EXIT_SUCCESS;
}
