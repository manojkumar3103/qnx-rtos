.PHONY: all clean

SUBDIRS = resource-manager interrupts ipc threads timers boot-image

all:
	@for dir in $(SUBDIRS); do $(MAKE) -C $$dir all; done

clean:
	@for dir in $(SUBDIRS); do $(MAKE) -C $$dir clean; done
