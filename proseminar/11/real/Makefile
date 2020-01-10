CC := gcc
CFLAGS := -O3 -g3
LDLIBS := -lm

.PHONY: all
all: real

objects := $(patsubst %.c,%.o,$(wildcard *.c))

*.o: *.c

real: $(objects)

.PHONY: clean
clean:
	$(RM) real *.o

.PHONY: run
run: all
	@echo "Sequential:"
	./real
	@echo
