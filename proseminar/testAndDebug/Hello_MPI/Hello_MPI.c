/* Hello World program with MPI framework */

#include <mpi.h>
#include <stdio.h>

int main(int argc, char **argv) {
	MPI_Init(&argc, &argv); // initialize the MPI framework
	int size;
	MPI_Comm_size(MPI_COMM_WORLD, &size); // get the number of ranks
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); // get the rank of the caller
	printf("Hello world from rank %d, of %d\n", rank, size);
	int number;
	if (rank == 0 && size > 1) {
		number = -1;
		MPI_Send(&number, 1, MPI_INT, 1, 42, MPI_COMM_WORLD);
	} else if (rank == 1) {
		MPI_Recv(&number, 1, MPI_INT, 0, 42, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("Rank 1: received %d from rank 0\n", number);
	}
	MPI_Finalize();
	return 0;
}
