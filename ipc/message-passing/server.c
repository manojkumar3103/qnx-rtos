/*
 * server.c - QNX message-passing server
 *
 * Creates a channel, receives string messages from clients,
 * computes a checksum, and replies with the result.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <process.h>

#include "msg_def.h"

int calculate_checksum(char *text);

int main(void)
{
	int        chid, pid;
	rcvid_t    rcvid;
	cksum_msg_t msg;
	int        status, checksum;

	chid = ChannelCreate(0);
	if (chid == -1) {
		perror("ChannelCreate");
		exit(EXIT_FAILURE);
	}

	pid = getpid();
	printf("Server pid: %d, chid: %d\n", pid, chid);

	while (1) {
		rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
		if (rcvid == -1) {
			perror("MsgReceive");
			exit(EXIT_FAILURE);
		}

		if (msg.msg_type == CKSUM_MSG_TYPE) {
			printf("Got checksum request\n");
			checksum = calculate_checksum(msg.string_to_cksum);

			status = MsgReply(rcvid, EOK, &checksum, sizeof(checksum));
			if (status == -1)
				perror("MsgReply");
		} else {
			printf("Unknown message type: %d\n", msg.msg_type);
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
