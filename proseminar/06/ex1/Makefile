CC := gcc
CFLAGS := -std=c99 -pedantic -D_DEFAULT_SOURCE -Wall -Wextra
LDLIBS := -lm -lz

.PHONY: all
all: particle_barnes_hut bin2png

particle_barnes_hut: particle_barnes_hut.o
particle_barnes_hut.o: particle_barnes_hut.c particle.h

bin2png: bin2png.o
bin2png.o: bin2png.c particle.h

.PHONY: clean
clean:
	$(RM) particle_barnes_hut bin2png *.o

.PHONY: rmdata
rmdata:
	$(RM) *.bin *.png *.gif