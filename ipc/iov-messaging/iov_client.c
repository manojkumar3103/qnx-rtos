/*
 * iov_client.c - Multi-part IOV message client
 *
 * Demonstrates sending a two-part message using IOV (scatter/gather):
 * Part 1 = header with message type and data size
 * Part 2 = the actual data string
 *
 * Usage: iov_client <string>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>

#include "iov_server.h"

int main(int argc, char *argv[])
{
	int            coid;
	cksum_header_t hdr;
	int            incoming_checksum;
	int            status;
	iov_t          siov[2];

	if (argc != 2) {
		printf("Usage: iov_client <string>\n");
		exit(EXIT_FAILURE);
	}

	coid = name_open(CKSUM_SERVER_NAME, 0);
	if (coid == -1) {
		perror("name_open");
		exit(EXIT_FAILURE);
	}

	printf("Sending: %s\n", argv[1]);

	hdr.msg_type  = CKSUM_IOV_MSG_TYPE;
	hdr.data_size = strlen(argv[1]) + 1;

	SETIOV(&siov[0], &hdr, sizeof(hdr));
	SETIOV(&siov[1], argv[1], hdr.data_size);

	status = MsgSendvs(coid, siov, 2, &incoming_checksum, sizeof(incoming_checksum));
	if (status == -1) {
		perror("MsgSendvs");
		exit(EXIT_FAILURE);
	}

	printf("Checksum: %d\n", incoming_checksum);
	return EXIT_SUCCESS;
}
