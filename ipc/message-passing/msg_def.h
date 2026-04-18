/*
 * msg_def.h - Message definitions for client-server checksum IPC
 */

#ifndef _MSG_DEF_H_
#define _MSG_DEF_H_

#include <sys/iomsg.h>

#define MAX_STRING_LEN  256
#define CKSUM_MSG_TYPE  (_IO_MAX + 1)

typedef struct {
	uint16_t msg_type;
	char     string_to_cksum[MAX_STRING_LEN + 1];
} cksum_msg_t;

#define CKSUM_PULSE_CODE  (_PULSE_CODE_MINAVAIL + 3)
#define SERVER_NAME       "cksum_server"
#define DISCONNECT_SERVER "disconnect_server"
#define UNBLOCK_SERVER    "unblock_server"

#endif
