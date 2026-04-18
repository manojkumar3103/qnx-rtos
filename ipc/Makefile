CC      = qcc
LD      = qcc
TARGET  = -Vgcc_ntox86_64
CFLAGS  += -g $(TARGET) -Wall
LDFLAGS += -g $(TARGET)

BINS = server client pulse_server pulse_client \
       name_server name_client \
       event_server event_client \
       shmem_creator shmem_user \
       shmem_qnx_server shmem_qnx_client \
       mutex_recovery \
       iov_server iov_client

all: $(BINS)

server: message-passing/server.c message-passing/msg_def.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

client: message-passing/client.c message-passing/msg_def.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

pulse_server: pulses/pulse_server.c message-passing/msg_def.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

pulse_client: pulses/pulse_client.c message-passing/msg_def.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

name_server: name-lookup/server.c message-passing/msg_def.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

name_client: name-lookup/client.c message-passing/msg_def.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

event_server: events/event_server.c events/event_server.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

event_client: events/event_client.c events/event_server.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

shmem_creator: shared-memory/posix/shmem_creator.c shared-memory/posix/shmem_posix.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

shmem_user: shared-memory/posix/shmem_user.c shared-memory/posix/shmem_posix.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

shmem_qnx_server: shared-memory/qnx-typed/shmem_server.c shared-memory/qnx-typed/shmem_qnx.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

shmem_qnx_client: shared-memory/qnx-typed/shmem_client.c shared-memory/qnx-typed/shmem_qnx.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

mutex_recovery: shared-memory/mutex-recovery/mutex_recovery.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

iov_server: iov-messaging/iov_server.c iov-messaging/iov_server.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

iov_client: iov-messaging/iov_client.c iov-messaging/iov_server.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f *.o $(BINS)
