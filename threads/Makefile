CC      = qcc
LD      = qcc
TARGET  = -Vgcc_ntox86_64
CFLAGS  += -g $(TARGET) -Wall
LDFLAGS += -g $(TARGET)

BINS = nomutex mutex_sync prodcons condvar condvar_queue spawn_example death_pulse

all: $(BINS)

nomutex: mutex-demo/nomutex.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

mutex_sync: mutex-demo/mutex_sync.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

prodcons: producer-consumer/prodcons.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

condvar: condvar/condvar.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

condvar_queue: condvar-queue/condvar_queue.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

spawn_example: process-spawn/spawn_example.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

death_pulse: death-notification/death_pulse.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f *.o $(BINS)
