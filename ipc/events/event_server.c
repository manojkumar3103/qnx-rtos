/*
 * event_server.c - Server-to-client event delivery via MsgDeliverEvent
 *
 * Demonstrates the QNX event notification pattern:
 * 1. Server registers a name via name_attach()
 * 2. Client sends a registration message containing a sigevent
 * 3. Server verifies the event with MsgVerifyEvent()
 * 4. A notify thread delivers the event to the client every second
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>

#include "event_server.h"

union recv_msgs {
	struct notification_request_msg client_msg;
	struct _pulse                   pulse;
	uint16_t                        type;
} recv_buf;

rcvid_t         save_rcvid = 0;
int             save_scoid = 0;
struct sigevent save_event;
int             notify_count = 0;
pthread_mutex_t save_data_mutex;

void *notify_thread(void *ignore);

int main(int argc, char *argv[])
{
	name_attach_t    *att;
	rcvid_t           rcvid;
	struct _msg_info  msg_info;
	int               status;

	att = name_attach(NULL, RECV_NAME, 0);
	if (att == NULL) {
		perror("name_attach");
		exit(EXIT_FAILURE);
	}

	status = pthread_mutex_init(&save_data_mutex, NULL);
	if (status != EOK) {
		fprintf(stderr, "pthread_mutex_init: %s\n", strerror(status));
		exit(EXIT_FAILURE);
	}

	status = pthread_create(NULL, NULL, notify_thread, NULL);
	if (status != EOK) {
		fprintf(stderr, "pthread_create: %s\n", strerror(status));
		exit(EXIT_FAILURE);
	}

	while (1) {
		rcvid = MsgReceive(att->chid, &recv_buf, sizeof(recv_buf), &msg_info);
		if (rcvid == -1) {
			perror("MsgReceive");
			exit(EXIT_FAILURE);
		}

		if (rcvid == 0) {
			switch (recv_buf.pulse.code) {
			case _PULSE_CODE_DISCONNECT:
				pthread_mutex_lock(&save_data_mutex);
				if (save_scoid == recv_buf.pulse.scoid) {
					save_scoid = 0;
					save_rcvid = 0;
					notify_count = 0;
				}
				pthread_mutex_unlock(&save_data_mutex);
				ConnectDetach(recv_buf.pulse.scoid);
				printf("Client disconnected: %#x\n", recv_buf.pulse.scoid);
				break;
			case _PULSE_CODE_UNBLOCK:
				printf("Unblock pulse received\n");
				MsgError(recv_buf.pulse.value.sival_long, -1);
				break;
			default:
				printf("Unexpected pulse code: %d\n", recv_buf.pulse.code);
				break;
			}
			continue;
		}

		switch (recv_buf.type) {
		case REQUEST_NOTIFICATIONS:
			if (MsgVerifyEvent(rcvid, &recv_buf.client_msg.ev) == -1) {
				perror("MsgVerifyEvent");
				MsgError(rcvid, EINVAL);
				continue;
			}

			pthread_mutex_lock(&save_data_mutex);
			save_rcvid = rcvid;
			save_event = recv_buf.client_msg.ev;
			save_scoid = msg_info.scoid;
			pthread_mutex_unlock(&save_data_mutex);

			if (MsgReply(rcvid, EOK, NULL, 0) == -1) {
				if (errno == ESRVRFAULT) {
					perror("MsgReply fatal");
					exit(EXIT_FAILURE);
				}
				perror("MsgReply");
			}
			printf("Client registered: %#lx\n", rcvid);
			break;
		default:
			printf("Unexpected message type: %d\n", recv_buf.type);
			MsgError(rcvid, ENOSYS);
			break;
		}
	}
	return EXIT_FAILURE;
}

void *notify_thread(void *ignore)
{
	int status;
	while (1) {
		sleep(1);

		status = pthread_mutex_lock(&save_data_mutex);
		if (status != EOK) {
			fprintf(stderr, "pthread_mutex_lock: %s\n", strerror(status));
			exit(EXIT_FAILURE);
		}

		if (save_rcvid) {
			printf("Delivering event to client %#lx\n", save_rcvid);

			if (save_event.sigev_notify & SIGEV_FLAG_UPDATEABLE)
				save_event.sigev_value.sival_int = notify_count++;

			if (MsgDeliverEvent(save_rcvid, &save_event) == -1) {
				perror("MsgDeliverEvent");
				if (errno == EFAULT)
					exit(EXIT_FAILURE);
			}
		}

		pthread_mutex_unlock(&save_data_mutex);
	}
	return NULL;
}
