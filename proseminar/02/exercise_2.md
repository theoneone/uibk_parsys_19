## Exercise 2

This exercise consists in parallelizing an application simulating the propagation of heat.

### Description

A large class of scientific applications are so-called stencil applications. These simulate time-dependent physical processes such as the propagation of heat or pressure in a given medium. The core of the simulation operates on a grid and updates each cell with information from its neighbor cells.

### Tasks

- A sequential implementation of a 1-D heat stencil is available in [heat_stencil_1D_seq.c](heat_stencil_1D/heat_stencil_1D_seq.c). Read the code and make sure you understand what happens. See the Wikipedia article on [Stencil Codes](https://en.wikipedia.org/wiki/Stencil_code) for more information.
	- the heat stencil simulates flow of properties along a given grid e.g. heat, pressure. The points of the grid are fixed. Each cell depends only on its neighbouring cells. The example code induces a fixed temperature at one cell and lets it propagate along a 1D chain. The result is a heat flow along the grid from the heat source towards both open ends.


- Consider a parallelization strategy using MPI. Which communication pattern(s) would you choose and why? Are there additional changes required in the code beyond calling MPI functions? If so, elaborate!
	- The major change is splitting the array into n pieces, where n is the number of ranks. The elements on both ends of each array depends on the neighboring element, which is placed at a neighbouring rank. So, those values need to be passed. I used a simple approach with a pair of `MPI_Send` and `MPI_Recv`. It is not performant.


- Implement your chosen parallelization strategy as a second application `heat_stencil_1D_mpi`. Run it with varying numbers of ranks and problem sizes and verify its correctness by comparing the output to `heat_stencil_1D_seq`.
	- Correctness is checked by comparing the sum of all elements and min/max values in the array. There is a bug in the algorithm, which occurs when the heat induction hits an edge of a minipatch. In this case a `continue` operation bypasses the code which handles message passing. So there is an additional change necessary.


- Discuss the effects and implications of your parallelization.
	- The choyce of `MPI_Send` and `MPI_Recv` was unlucky, so the processes slow down extremely with increasing number of ranks.

