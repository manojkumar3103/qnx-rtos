/*
 * server.c - Name-lookup based checksum server
 *
 * Registers with name_attach() so clients can discover the server
 * by name instead of requiring pid/chid. Handles pulses, messages,
 * and disconnect notifications.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <process.h>

#include "../message-passing/msg_def.h"

int calculate_checksum(char *text);

typedef union {
	uint16_t        type;
	cksum_msg_t     cksum_msg;
	struct _pulse   pulse;
} recv_buf_t;

int main(void)
{
	rcvid_t      rcvid;
	recv_buf_t   rbuf;
	int          status, checksum;
	name_attach_t *att;

	att = name_attach(NULL, SERVER_NAME, 0);
	if (att == NULL) {
		perror("name_attach");
		exit(EXIT_FAILURE);
	}

	printf("Server pid: %d, chid: %d\n", getpid(), att->chid);

	while (1) {
		rcvid = MsgReceive(att->chid, &rbuf, sizeof(rbuf), NULL);
		if (rcvid == -1) {
			perror("MsgReceive");
			exit(EXIT_FAILURE);
		}

		if (rcvid == 0) {
			switch (rbuf.pulse.code) {
			case _PULSE_CODE_DISCONNECT:
				ConnectDetach(rbuf.pulse.scoid);
				break;
			case CKSUM_PULSE_CODE:
				printf("Received CKSUM pulse\n");
				break;
			default:
				printf("Unexpected pulse code: %d\n", rbuf.pulse.code);
				break;
			}
			continue;
		}

		switch (rbuf.type) {
		case CKSUM_MSG_TYPE:
			printf("Got checksum request\n");
			checksum = calculate_checksum(rbuf.cksum_msg.string_to_cksum);

			status = MsgReply(rcvid, EOK, &checksum, sizeof(checksum));
			if (status == -1)
				perror("MsgReply");
			break;
		default:
			if (MsgError(rcvid, ENOSYS) == -1)
				perror("MsgError");
			break;
		}
	}
	return 0;
}

int calculate_checksum(char *text)
{
	int cksum = 0;
	for (char *c = text; *c != '\0'; c++)
		cksum += *c;
	return cksum;
}
