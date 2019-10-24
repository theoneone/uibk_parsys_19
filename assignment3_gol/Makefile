CFLAGS=-O2 -std=c99 -Wall -Wextra -pedantic

heat_stencil_2D_seq: heat_stencil_2D_seq.o gen_image.o

heat_stencil_2D_seq.o: heat_stencil_2D_seq.c heat_stencil.h
gen_image.o: gen_image.c heat_stencil.h

.PHONY: clean
clean:
	$(RM) heat_stencil_2D_seq *.o
