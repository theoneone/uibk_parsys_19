OPT :=-O0
CC :=gcc
CFLAGS :=-std=c99 -pedantic -D_DEFAULT_SOURCE -Wall -Wextra $(OPT)
LDLIBS :=-lm

.PHONY: all
all: queens

queens: queens.o
queens.o: queens.c

.PHONY: clean
clean:
	$(RM) queens *.o
