/*
 * event_server.h - Event notification protocol definitions
 */

#ifndef _EVENT_SERVER_H_
#define _EVENT_SERVER_H_

#include <sys/siginfo.h>
#include <stdint.h>

#define RECV_NAME "MSG_RECEIVER"

struct notification_request_msg {
	uint16_t        type;
	struct sigevent ev;
};

#define REQUEST_NOTIFICATIONS (_IO_MAX + 100)

#endif
