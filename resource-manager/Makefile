CC      = qcc
LD      = qcc
TARGET  = -Vgcc_ntox86_64
CFLAGS  += -g $(TARGET) -Wall
LDFLAGS += -g $(TARGET) -lsecpol

BINS = example

all: $(BINS)

clean:
	rm -f *.o $(BINS)
