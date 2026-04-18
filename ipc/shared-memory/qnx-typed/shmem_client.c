/*
 * shmem_client.c - QNX typed shared memory client
 *
 * Requests a shared memory handle from the server, maps it locally,
 * writes data, notifies the server, reads the response, and cleans up.
 *
 * Usage: shmem_client [string_to_send]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>

#include "shmem_qnx.h"

#define DEFAULT_CLIENT_STRING "hello from client"

int main(int argc, char **argv)
{
	int                  coid, status, mem_fd;
	unsigned             len;
	char                *mem_ptr;
	get_shmem_msg_t      get_msg;
	get_shmem_resp_t     get_resp;
	release_shmem_msg_t  release_msg;
	changed_shmem_msg_t  changed_msg;
	changed_shmem_resp_t changed_resp;

	coid = name_open(SHMEM_SERVER_NAME, 0);
	if (coid == -1) {
		perror("name_open");
		exit(EXIT_FAILURE);
	}

	get_msg.type = GET_SHMEM_MSG_TYPE;
	get_msg.shared_mem_bytes = 8192;

	status = MsgSend(coid, &get_msg, sizeof(get_msg), &get_resp, sizeof(get_resp));
	if (status == -1) {
		perror("MsgSend");
		exit(EXIT_FAILURE);
	}

	mem_fd = shm_open_handle(get_resp.mem_handle, O_RDWR);
	if (mem_fd == -1) {
		perror("shm_open_handle");
		exit(EXIT_FAILURE);
	}

	mem_ptr = mmap(NULL, 8192, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, 0);
	if (mem_ptr == MAP_FAILED) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}
	close(mem_fd);

	if (argc > 1) {
		len = strlen(argv[1]);
		memcpy(mem_ptr + 20, argv[1], len);
	} else {
		len = sizeof(DEFAULT_CLIENT_STRING) - 1;
		memcpy(mem_ptr + 20, DEFAULT_CLIENT_STRING, len);
	}

	changed_msg.type   = CHANGED_SHMEM_MSG_TYPE;
	changed_msg.offset = 20;
	changed_msg.length = len;

	status = MsgSend(coid, &changed_msg, sizeof(changed_msg),
	                 &changed_resp, sizeof(changed_resp));
	if (status == -1) {
		perror("MsgSend");
		exit(EXIT_FAILURE);
	}

	printf("Server response:\n");
	write(STDOUT_FILENO, mem_ptr + changed_resp.offset, changed_resp.length);
	write(STDOUT_FILENO, "\n", 1);

	munmap(mem_ptr, 8192);
	release_msg.type = RELEASE_SHMEM_MSG_TYPE;
	MsgSend(coid, &release_msg, sizeof(release_msg), NULL, 0);

	return EXIT_SUCCESS;
}
