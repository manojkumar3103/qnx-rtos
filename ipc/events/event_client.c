/*
 * event_client.c - Client receiving periodic events from server
 *
 * Finds the server via name_open(), creates a private channel for
 * pulse delivery, registers a sigevent with the server, then loops
 * receiving pulse notifications.
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/siginfo.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>

#include "event_server.h"

#define MY_PULSE_CODE (_PULSE_CODE_MINAVAIL + 3)

union recv_msg {
	struct _pulse pulse;
	short         type;
} recv_buf;

int server_locate()
{
	int coid;
	coid = name_open(RECV_NAME, 0);
	while (coid == -1) {
		sleep(1);
		coid = name_open(RECV_NAME, 0);
	}
	return coid;
}

int main(int argc, char *argv[])
{
	int                            server_coid, self_coid, chid;
	rcvid_t                        rcvid;
	struct notification_request_msg msg;
	struct sched_param             sched_param;

	chid = ChannelCreate(_NTO_CHF_PRIVATE);
	if (chid == -1) {
		perror("ChannelCreate");
		exit(EXIT_FAILURE);
	}

	server_coid = server_locate();

	self_coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0);
	if (self_coid == -1) {
		perror("ConnectAttach");
		exit(EXIT_FAILURE);
	}

	msg.type = REQUEST_NOTIFICATIONS;

	pthread_getschedparam(0, NULL, &sched_param);
	SIGEV_PULSE_INIT(&msg.ev, self_coid, sched_param.sched_priority, MY_PULSE_CODE, 0);
	SIGEV_MAKE_UPDATEABLE(&msg.ev);

	if (MsgRegisterEvent(&msg.ev, server_coid) == -1) {
		perror("MsgRegisterEvent");
		exit(EXIT_FAILURE);
	}

	if (MsgSend(server_coid, &msg, sizeof(msg), NULL, 0) == -1) {
		perror("MsgSend");
		exit(EXIT_FAILURE);
	}

	while (1) {
		rcvid = MsgReceive(chid, &recv_buf, sizeof(recv_buf), NULL);
		if (rcvid == -1) {
			perror("MsgReceive");
			exit(EXIT_FAILURE);
		}

		if (rcvid == 0) {
			if (recv_buf.pulse.code == MY_PULSE_CODE)
				printf("Event received, value = %d\n", recv_buf.pulse.value.sival_int);
			else
				printf("Unexpected pulse code: %d\n", recv_buf.pulse.code);
			continue;
		}

		printf("Unexpected message type: %d\n", recv_buf.type);
		MsgError(rcvid, ENOSYS);
	}
}
