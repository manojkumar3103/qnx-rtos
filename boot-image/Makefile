%.ifs : %.build
	mkifs -v $< $@

IMAGES = training_image.ifs

all: $(IMAGES)

clean:
	rm -f $(IMAGES) *.sym
