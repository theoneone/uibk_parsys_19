CC=gcc
CC_FLAGS=-O3 -g3 -lm -fopenmp -march=native -mtune=native

.PHONEY: all
all: real

SRC=$(wildcard *.c)

real: $(SRC)
	@$(CC) $(CC_FLAGS) $(SRC) -o real

.PHONEY: clean
clean:
	@rm real

.PHONEY: run
run: all
	@echo "Sequential:"
	@./real
	@echo
