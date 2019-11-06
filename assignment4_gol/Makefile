CFLAGS := -std=c99 -pedantic -Wall -Wextra
LDFLAGS := -lm

particle: particle.o
particle.o: particle.c particle.h

.PHONY: clean
clean:
	$(RM) particle *.o
