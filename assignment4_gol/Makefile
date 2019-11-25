CFLAGS := -std=c99 -pedantic -D_DEFAULT_SOURCE -Wall -Wextra -fopenmp
LOADLIBES := -lm -lz -lgomp

.PHONY: all
all: particle bin2png

particle: particle.o
particle.o: particle.c particle.h

bin2png: bin2png.o
bin2png.o: bin2png.c particle.h

.PHONY: clean
clean:
	$(RM) particle bin2png *.o
