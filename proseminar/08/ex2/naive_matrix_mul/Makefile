CC := gcc
CFLAGS := -std=c99 -pedantic -D_DEFAULT_SOURCE -Wall -Wextra
LDLIBS := -lm -lz

.PHONY: all
all: matrix_mul

matrix_mul: matrix_mul.o
matrix_mul.o: matrix_mul.c

.PHONY: clean
clean:
	$(RM) matrix_mul *.o

