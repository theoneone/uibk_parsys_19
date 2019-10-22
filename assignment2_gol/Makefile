CFLAGS := -std=c99 -pedantic -Wall -Wextra -D_GNU_SOURCE
LDFLAGS := -lm

.PHONY: all
all: pi_test pi_eval

.PHONY: clean
clean:
	$(RM) pi_test pi_eval

pi_eval: pi_eval.c
pi_test: pi_test.c
