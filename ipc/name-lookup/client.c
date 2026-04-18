/*
 * client.c - Name-lookup based checksum client
 *
 * Uses name_open() to locate the server by name, sends a pulse
 * followed by a checksum request message.
 *
 * Usage: client <string>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>

#include "../message-passing/msg_def.h"

int main(int argc, char *argv[])
{
	int         coid;
	cksum_msg_t msg;
	int         incoming_checksum;
	int         status;

	if (argc != 2) {
		printf("Usage: client <string>\n");
		exit(EXIT_FAILURE);
	}

	coid = name_open(SERVER_NAME, 0);
	if (coid == -1) {
		perror("name_open");
		exit(EXIT_FAILURE);
	}

	msg.msg_type = CKSUM_MSG_TYPE;
	strlcpy(msg.string_to_cksum, argv[1], sizeof(msg.string_to_cksum));
	printf("Sending: %s\n", msg.string_to_cksum);

	if (MsgSendPulse(coid, -1, CKSUM_PULSE_CODE, 0xc0dedeed) == -1)
		perror("MsgSendPulse");

	status = MsgSend(coid, &msg, sizeof(msg.msg_type) + strlen(msg.string_to_cksum) + 1,
	                 &incoming_checksum, sizeof(incoming_checksum));
	if (status == -1) {
		perror("MsgSend");
		exit(EXIT_FAILURE);
	}

	printf("Checksum: %d\n", incoming_checksum);
	return EXIT_SUCCESS;
}
