OPT :=-O2
CC := gcc
CFLAGS :=-std=c99 -pedantic -D_DEFAULT_SOURCE -Wall -Wextra $(OPT)
LDLIBS :=-lm

.PHONY: all
all: matrix_mul

matrix_mul: matrix_mul.o
matrix_mul.o: matrix_mul.c

.PHONY: profile
profile: CFLAGS +=-g -pg
profile: LDFLAGS +=-pg
profile: clean all
	./matrix_mul -c 10000 -r 10000 -b 10000
	gprof ./matrix_mul

.PHONY: clean
clean:
	$(RM) matrix_mul *.o gmon.out
