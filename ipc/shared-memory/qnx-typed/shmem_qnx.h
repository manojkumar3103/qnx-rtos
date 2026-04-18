/*
 * shmem_qnx.h - QNX typed shared memory protocol definitions
 */

#ifndef _SHMEM_QNX_H_
#define _SHMEM_QNX_H_

#include <stdint.h>
#include <sys/iomsg.h>
#include <sys/mman.h>

#define GET_SHMEM_MSG_TYPE     (_IO_MAX + 200)
#define CHANGED_SHMEM_MSG_TYPE (_IO_MAX + 201)
#define RELEASE_SHMEM_MSG_TYPE (_IO_MAX + 202)

#define SHMEM_SERVER_NAME "shmem_server"

typedef struct {
	uint16_t type;
	unsigned shared_mem_bytes;
} get_shmem_msg_t;

typedef struct {
	shm_handle_t mem_handle;
} get_shmem_resp_t;

typedef struct {
	uint16_t type;
	unsigned offset;
	unsigned length;
} changed_shmem_msg_t;

typedef struct {
	unsigned offset;
	unsigned length;
} changed_shmem_resp_t;

typedef struct {
	uint16_t type;
} release_shmem_msg_t;

#endif
