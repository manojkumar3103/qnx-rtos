CC      = qcc
LD      = qcc
TARGET  = -Vgcc_ntox86_64
CFLAGS  += -g $(TARGET) -Wall
LDFLAGS += -g $(TARGET)

BINS = interrupt_event interrupt_thread

all: $(BINS)

clean:
	rm -f *.o $(BINS)
