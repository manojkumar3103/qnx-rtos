CC      = qcc
LD      = qcc
TARGET  = -Vgcc_ntox86_64
CFLAGS  += -g $(TARGET) -Wall
LDFLAGS += -g $(TARGET)

BINS = reptimer nonblockpulserec

all: $(BINS)

clean:
	rm -f *.o $(BINS)
