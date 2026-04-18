/*
 * iov_server.h - IOV messaging protocol definitions
 */

#ifndef _IOV_SERVER_H_
#define _IOV_SERVER_H_

#include <sys/iomsg.h>

#define CKSUM_SERVER_NAME  "cksum"
#define CKSUM_IOV_MSG_TYPE (_IO_MAX + 2)

typedef struct {
	uint16_t msg_type;
	unsigned data_size;
} cksum_header_t;

#endif
