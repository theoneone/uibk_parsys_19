CC=gcc
CC_FLAGS=-O2 -std=c99 -Wall -Wextra -pedantic
NAME=heat_stencil_1D_seq

.PHONEY: all
all: $(NAME)

$(NAME): $(NAME).c
	@$(CC) $(CC_FLAGS) $< -o $(NAME)

.PHONEY: clean
clean:
	@rm $(NAME)

.PHONEY: run
run: all
	@echo "Sequential:"
	@./$(NAME)
	@echo
