CC=mpicc
CC_FLAGS=-O2 -std=c99 -Wall -Wextra -pedantic
NAME=heat_stencil_1D_mpi

.PHONEY: all
all: $(NAME)

$(NAME): $(NAME).c
	@$(CC) $(CC_FLAGS) $< -o $(NAME)

.PHONEY: clean
clean:
	@rm $(NAME)

.PHONEY: run
run: all
		@echo "MPI parallel:"
	@mpiexec -n 4 $(NAME)
		@echo
