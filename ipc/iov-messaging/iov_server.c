/*
 * iov_server.c - Multi-part IOV message server
 *
 * Demonstrates receiving messages with separate header and data parts
 * using MsgRead() to retrieve data beyond what fits in the initial receive.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/dispatch.h>
#include <sys/neutrino.h>

#include "iov_server.h"

int calculate_checksum(char *text);

typedef union {
	uint16_t       msg_type;
	struct _pulse  pulse;
	cksum_header_t cksum_hdr;
} msg_buf_t;

int main(void)
{
	rcvid_t          rcvid;
	name_attach_t   *attach;
	msg_buf_t        msg;
	int              status, checksum;
	char            *data;
	struct _msg_info minfo;

	attach = name_attach(NULL, CKSUM_SERVER_NAME, 0);
	if (attach == NULL) {
		perror("name_attach");
		exit(EXIT_FAILURE);
	}

	while (1) {
		printf("Waiting for message...\n");
		rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), &minfo);
		if (rcvid == -1) {
			perror("MsgReceive");
			exit(EXIT_FAILURE);
		}

		if (rcvid > 0) {
			if (minfo.msglen < sizeof(msg.msg_type)) {
				MsgError(rcvid, EBADMSG);
				continue;
			}

			switch (msg.msg_type) {
			case CKSUM_IOV_MSG_TYPE:
				if (minfo.msglen < sizeof(msg.cksum_hdr)) {
					MsgError(rcvid, EBADMSG);
					continue;
				}
				printf("Checksum request: %d bytes of data\n", msg.cksum_hdr.data_size);

				if (minfo.srcmsglen < sizeof(msg.cksum_hdr) + msg.cksum_hdr.data_size) {
					MsgError(rcvid, EBADMSG);
					continue;
				}

				data = malloc(msg.cksum_hdr.data_size);
				if (data == NULL) {
					MsgError(rcvid, ENOMEM);
					continue;
				}

				status = MsgRead(rcvid, data, msg.cksum_hdr.data_size, sizeof(cksum_header_t));
				if (status == -1) {
					int save_errno = errno;
					perror("MsgRead");
					MsgError(rcvid, save_errno);
					free(data);
					continue;
				}

				checksum = calculate_checksum(data);
				free(data);

				status = MsgReply(rcvid, EOK, &checksum, sizeof(checksum));
				if (status == -1) {
					if (errno == ESRVRFAULT) {
						perror("MsgReply fatal");
						exit(EXIT_FAILURE);
					}
					perror("MsgReply");
				}
				break;
			default:
				MsgError(rcvid, ENOSYS);
				break;
			}
		} else {
			switch (msg.pulse.code) {
			case _PULSE_CODE_DISCONNECT:
				ConnectDetach(msg.pulse.scoid);
				break;
			case _PULSE_CODE_UNBLOCK:
				MsgError(msg.pulse.value.sival_long, -1);
				break;
			default:
				printf("Unknown pulse code: %d\n", msg.pulse.code);
				break;
			}
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
