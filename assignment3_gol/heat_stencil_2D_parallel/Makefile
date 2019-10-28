CC=mpicc
CFLAGS=-O2 -std=c99 -Wall -Wextra -pedantic
NAME=heat_stencil_2D_mpi

.PHONY: all
all: $(NAME)

$(NAME): $(NAME).o gen_image.o
$(NAME).o: $(NAME).c ../heat_stencil.h
gen_image.o: ../gen_image.c ../heat_stencil.h
	$(MAKE) -C .. gen_image.o
	cp ../gen_image.o gen_image.o

.PHONY: clean
clean:
	$(RM) $(NAME) *.o

.PHONY: run
run: all
		@echo "MPI parallel:"
	@mpiexec -n 4 $(NAME)
		@echo
