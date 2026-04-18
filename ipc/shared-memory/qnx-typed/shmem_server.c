/*
 * shmem_server.c - QNX typed shared memory server
 *
 * Uses shm_create_handle() to securely share anonymous memory with a client.
 * The server creates the shared memory, provides a handle to the client,
 * processes change notifications, and cleans up on disconnect.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <process.h>

#include "shmem_qnx.h"

#define DEFAULT_RESPONSE "Answer from server"

int create_shared_memory(unsigned nbytes, int client_pid, void **ptr, shm_handle_t *handle)
{
	int fd = shm_open(SHM_ANON, O_RDWR | O_CREAT, 0600);
	if (fd == -1) {
		perror("shm_open");
		return -1;
	}

	if (ftruncate(fd, nbytes) == -1) {
		perror("ftruncate");
		close(fd);
		return -1;
	}

	*ptr = mmap(NULL, nbytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (*ptr == MAP_FAILED) {
		perror("mmap");
		close(fd);
		return -1;
	}

	if (shm_create_handle(fd, client_pid, O_RDWR, handle, 0) == -1) {
		perror("shm_create_handle");
		close(fd);
		munmap(*ptr, nbytes);
		return -1;
	}

	close(fd);
	return 0;
}

typedef union {
	uint16_t            type;
	struct _pulse       pulse;
	get_shmem_msg_t     get_shmem;
	changed_shmem_msg_t changed_shmem;
} recv_buf_t;

int main(int argc, char **argv)
{
	rcvid_t              rcvid;
	recv_buf_t           rbuf;
	int                  status;
	name_attach_t       *att;
	struct _msg_info     msg_info;
	int                  client_scoid = 0;
	char                *shmem_ptr;
	unsigned             shmem_size;
	get_shmem_resp_t     get_resp;
	changed_shmem_resp_t changed_resp;
	char                *resp;
	unsigned             resp_len;

	if (argc > 1) {
		resp = argv[1];
		resp_len = strlen(resp);
	} else {
		resp = DEFAULT_RESPONSE;
		resp_len = sizeof(DEFAULT_RESPONSE) - 1;
	}

	att = name_attach(NULL, SHMEM_SERVER_NAME, 0);
	if (att == NULL) {
		perror("name_attach");
		exit(EXIT_FAILURE);
	}

	while (1) {
		rcvid = MsgReceive(att->chid, &rbuf, sizeof(rbuf), &msg_info);

		if (rcvid == -1) {
			perror("MsgReceive");
			exit(EXIT_FAILURE);
		}

		if (rcvid == 0) {
			if (rbuf.pulse.code == _PULSE_CODE_DISCONNECT) {
				if (rbuf.pulse.scoid == client_scoid) {
					client_scoid = 0;
					munmap(shmem_ptr, shmem_size);
				}
				ConnectDetach(rbuf.pulse.scoid);
			}
			continue;
		}

		switch (rbuf.type) {
		case GET_SHMEM_MSG_TYPE:
			shmem_size = rbuf.get_shmem.shared_mem_bytes;
			if (shmem_size > 64 * 1024) {
				MsgError(rcvid, EINVAL);
				continue;
			}
			status = create_shared_memory(shmem_size, msg_info.pid,
			                              (void *)&shmem_ptr, &get_resp.mem_handle);
			if (status != 0) {
				MsgError(rcvid, errno);
				continue;
			}
			client_scoid = msg_info.scoid;

			status = MsgReply(rcvid, EOK, &get_resp, sizeof(get_resp));
			if (status == -1) {
				perror("MsgReply");
				MsgError(rcvid, errno);
			}
			break;

		case CHANGED_SHMEM_MSG_TYPE: {
			if (msg_info.scoid != client_scoid) {
				MsgError(rcvid, EPERM);
				continue;
			}

			const unsigned offset = rbuf.changed_shmem.offset;
			const unsigned nbytes = rbuf.changed_shmem.length;
			if (nbytes > shmem_size || offset > shmem_size ||
			    (nbytes + offset) > shmem_size) {
				MsgError(rcvid, EBADMSG);
				continue;
			}

			printf("Client data:\n");
			write(STDOUT_FILENO, shmem_ptr + offset, nbytes);
			write(STDOUT_FILENO, "\n", 1);

			changed_resp.offset = 4096 + 30;
			memcpy(shmem_ptr + changed_resp.offset, resp, resp_len);
			changed_resp.length = resp_len;

			status = MsgReply(rcvid, EOK, &changed_resp, sizeof(changed_resp));
			if (status == -1) {
				perror("MsgReply");
				MsgError(rcvid, errno);
			}
			break;
		}

		case RELEASE_SHMEM_MSG_TYPE:
			if (msg_info.scoid != client_scoid) {
				MsgError(rcvid, EPERM);
				continue;
			}
			client_scoid = 0;
			munmap(shmem_ptr, shmem_size);
			MsgReply(rcvid, EOK, NULL, 0);
			break;

		default:
			MsgError(rcvid, ENOSYS);
			break;
		}
	}
	return 0;
}
