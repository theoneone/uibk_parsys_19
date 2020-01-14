OPT :=-O0
PROF_OUT := profile.txt
CC := gcc
CFLAGS := -fopenmp -pedantic -Wall -Wextra $(OPT)
LDLIBS := -lm -lgomp

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

.PHONY: profile
profile: CFLAGS +=-g3 -pg
profile: LDFLAGS +=-pg
profile: clean all
	./real
	gprof ./real > $(PROF_OUT)
