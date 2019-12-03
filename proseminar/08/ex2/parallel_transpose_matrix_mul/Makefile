OPT :=-O0
CC :=gcc
CFLAGS :=-std=c99 -pedantic -D_DEFAULT_SOURCE -Wall -Wextra -fopenmp $(OPT)
LDLIBS :=-lm -lgomp
ifneq ($(strip $(OUT)),)
    OUT1 := >1k_$(OUT)
    OUT2 := >5k_$(OUT)
endif

.PHONY: all
all: matrix_mul

matrix_mul: matrix_mul.o
matrix_mul.o: matrix_mul.c

.PHONY: profile
profile: CFLAGS +=-g -pg
profile: LDFLAGS +=-pg
profile: clean all
	./matrix_mul -c 1000 -r 1000 -b 1000
	gprof ./matrix_mul $(OUT1)
	./matrix_mul -c 1000 -r 1000 -b 5000
	gprof ./matrix_mul $(OUT2)

.PHONY: clean
clean:
	$(RM) matrix_mul *.o gmon.out

