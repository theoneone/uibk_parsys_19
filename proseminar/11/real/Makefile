CC=gcc
CFLAGS=-O3 -g3 -lm

.PHONY: all
all: real

SRC=$(wildcard *.c)

real: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o real

.PHONY: clean
clean:
	$(RM) real

.PHONY: run
run: all
	@echo "Sequential:"
	./real
	@echo
