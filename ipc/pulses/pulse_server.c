/*
 * pulse_server.c - Server receiving both pulses and messages
 *
 * Extends the basic server to handle pulses alongside standard
 * message-passing. Uses a union receive buffer to distinguish
 * between pulse and message types.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <process.h>

#include "../message-passing/msg_def.h"

int calculate_checksum(char *text);

typedef union {
	uint16_t      type;
	cksum_msg_t   cksum_msg;
	struct _pulse pulse;
} recv_buf_t;

int main(void)
{
	int        chid, pid;
	rcvid_t    rcvid;
	recv_buf_t rbuf;
	int        status, checksum;

	chid = ChannelCreate(0);
	if (chid == -1) {
		perror("ChannelCreate");
		exit(EXIT_FAILURE);
	}

	pid = getpid();
	printf("Server pid: %d, chid: %d\n", pid, chid);

	while (1) {
		rcvid = MsgReceive(chid, &rbuf, sizeof(rbuf), NULL);
		if (rcvid == -1) {
			perror("MsgReceive");
			exit(EXIT_FAILURE);
		}

		if (rcvid == 0) {
			printf("Pulse received: code=%d, value=%#lx\n",
			       rbuf.pulse.code, rbuf.pulse.value.sival_long);
			continue;
		}

		if (rbuf.type == CKSUM_MSG_TYPE) {
			printf("Got checksum request\n");
			checksum = calculate_checksum(rbuf.cksum_msg.string_to_cksum);

			status = MsgReply(rcvid, EOK, &checksum, sizeof(checksum));
			if (status == -1)
				perror("MsgReply");
		} else {
			if (MsgError(rcvid, ENOSYS) == -1)
				perror("MsgError");
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
