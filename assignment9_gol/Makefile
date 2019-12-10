OPT :=-O3 -Ofast -fopenmp
CC :=gcc
CFLAGS :=-std=c99 -pedantic -D_DEFAULT_SOURCE -Wall -Wextra $(OPT)
LDLIBS := -lomp

.PHONY: all
all: queens queens_ser

queens: queens.o
queens_ser: queens_ser.o

queens.o: queens.c
queens_ser.o: queens_ser.c

.PHONY: clean
clean:
	$(RM) queens queens_ser *.o
