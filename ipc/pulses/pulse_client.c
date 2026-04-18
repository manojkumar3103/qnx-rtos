/*
 * pulse_client.c - Client demonstrating pulse and message sending
 *
 * Sends a pulse (MsgSendPulse) followed by a standard message
 * to the server. Shows how both IPC mechanisms coexist.
 *
 * Usage: pulse_client <server_pid> <server_chid> <string>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/neutrino.h>

#include "../message-passing/msg_def.h"

int main(int argc, char *argv[])
{
	int         coid;
	cksum_msg_t msg;
	int         incoming_checksum;
	int         status;

	if (argc != 4) {
		printf("Usage: pulse_client <server_pid> <server_chid> <string>\n");
		exit(EXIT_FAILURE);
	}

	int server_pid  = atoi(argv[1]);
	int server_chid = atoi(argv[2]);

	printf("Connecting to server pid: %d, chid: %d\n", server_pid, server_chid);

	coid = ConnectAttach(0, server_pid, server_chid, _NTO_SIDE_CHANNEL, 0);
	if (coid == -1) {
		perror("ConnectAttach");
		exit(EXIT_FAILURE);
	}

	msg.msg_type = CKSUM_MSG_TYPE;
	strlcpy(msg.string_to_cksum, argv[3], sizeof(msg.string_to_cksum));
	printf("Sending: %s\n", msg.string_to_cksum);

	status = MsgSendPulse(coid, -1, 3, 0xdeadc0de);
	if (status == -1)
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
