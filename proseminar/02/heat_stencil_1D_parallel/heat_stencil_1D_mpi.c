#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

typedef double value_t;

#define RESOLUTION 120

// -- vector utilities --

typedef value_t *Vector;
typedef int *IVector;

Vector createVector(int N);

void releaseVector(Vector m);

IVector createIVector(int N);

void releaseIVector(IVector m);

void printTemperature(Vector m, int N);

value_t sumVector(Vector V, int N, value_t *min, value_t *max);

int calculate_part(int size, int rank, int N, int T);

// -- simulation code ---

int main(int argc, char **argv) {
	MPI_Init(&argc, &argv);
	int size;
	MPI_Comm_size(MPI_COMM_WORLD, &size); // get the number of ranks
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); // get the rank of the caller

	// 'parsing' optional input parameter = problem size
	int N = 2000;
	if (argc > 1) {
		N = atoi(argv[1]);
	}
	int T = N * 500;

	int success = 1;
	if (rank == 0) {
		printf("Computing heat-distribution for room size N=%d for T=%d timesteps\n", N, T);
	}
	success = calculate_part(size, rank, N, T);

	MPI_Finalize();
	// done
	return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}

Vector createVector(int N) {
	// create data and index vector
	return malloc(sizeof(value_t) * N);
}

void releaseVector(Vector m) { free(m); }

IVector createIVector(int N) {
	// create data and index vector
	return malloc(sizeof(int) * N);
}

void releaseIVector(IVector m) { free(m); }

void printTemperature(Vector m, int N) {
	const char *colors = " .-:=+*^X#%@";
	const int numColors = 12;

	// boundaries for temperature (for simplicity hard-coded)
	const value_t max = 273 + 30;
	const value_t min = 273 + 0;

	// set the 'render' resolution
	int W = RESOLUTION;

	// step size in each dimension
	int sW = N / W;

	// room
	// left wall
	printf("X");
	// actual room
	for (int i = 0; i < W; i++) {
		// get max temperature in this tile
		value_t max_t = 0;
		for (int x = sW * i; x < sW * i + sW; x++) {
			max_t = (max_t < m[x]) ? m[x] : max_t;
		}
		value_t temp = max_t;

		// pick the 'color'
		int c = ((temp - min) / (max - min)) * numColors;
		c = (c >= numColors) ? numColors - 1 : ((c < 0) ? 0 : c);

		// print the average temperature
		printf("%c", colors[c]);
	}
	// right wall
	printf("X");
}

value_t sumVector(Vector V, int N, value_t *min, value_t *max) {
	value_t result = 0.0;
	if (min != NULL) *min = V[0];
	if (max != NULL) *max = V[0];

	for (int i = 0; i < N; ++i) {
		result += V[i];
		if(min != NULL && V[i] < *min) *min = V[i];
		if(max != NULL && V[i] > *max) *max = V[i];
	}
	return result;
}

int calculate_part(int size, int rank, int N_elements, int T) {
	// ---------- setup ----------
	int N = (N_elements + size-1) / size;
	IVector rcv_counts, rcv_displs;
	rcv_counts = createIVector(size);
	rcv_displs = createIVector(size);
	int displ = 0;
	for (int i = 0; i < size; ++i) {
		rcv_counts[i] = N;
		rcv_displs[i] = displ;
		displ += N;
	}
	printf("rank: %d, N = %d, N_el = %d, last=%d\n", rank, N, N_elements, N_elements % N);//XXX
	rcv_counts[size-1] = N_elements % N == 0 ? N : N_elements % N;
	if (rank == size-1) {
		N = rcv_counts[rank]; // last rank might be not entirely filled, remove unnecessary elements
	}
	printf("cnt: ");//XXX 7 Zeilen
	for(int i=0; i<size;++i) printf("%d, ", rcv_counts[i]);
	printf("\n");
	printf("dsp: ");
	for(int i=0; i<size;++i) printf("%d, ", rcv_displs[i]);
	printf(" %d\n", rcv_counts[size-1]+rcv_displs[size-1]);
	printf("rank %d, N=%d\n", rank, N);

	// create a buffer A for storing temperature fields
	// create a second buffer B for the computation
	Vector A, B;
	int NN; // length of A (N is length considered for calculation)
	if (rank == 0) {
		A = createVector(N_elements);
		B = createVector(N_elements);
		NN = N_elements;
	} else {
		A = createVector(N);
		B = createVector(N);
		NN = N;
	}
	// set up initial conditions in A
	for (int i = 0; i < NN; i++) {
		A[i] = 273; // temperature is 0° C everywhere (273 K)
	}
	// and there is a heat source in one corner
	int source_x = -1; // negative value: source not in this rank
	if (rcv_displs[rank] <= N_elements / 4 && rcv_displs[rank] + N > N_elements / 4) {
		source_x = (N_elements / 4) % N;
		A[source_x] = 273 + 60;
		printf("rank %d, seed at %d, heat = %f\n", rank, source_x, A[source_x]);
	}
	printf("rank %d, seed at %d (find source array)\n", rank, source_x);
	if (rank == 0) {
		printf("Initial:\t");
		printTemperature(A, N_elements);
		printf("\n");
	}
	// ---------- compute ----------
	// for each time step ..
	for (int t = 0; t < T; t++) {
		if (rank != 0) {
			if(MPI_Send(&(A[0]), 1, MPI_DOUBLE, rank-1, 42, MPI_COMM_WORLD) != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Send failed (left) at rank %d.\n", rank);
			}
		}
		if (rank != size-1) {
			if(MPI_Send(&(A[N-1]), 1, MPI_DOUBLE, rank+1, 42, MPI_COMM_WORLD) != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Send failed (right) at rank %d.\n", rank);
			}
		}
		// .. we propagate the temperature
		for (long long i = 0; i < N; i++) {
			// center stays constant (the heat is still on)
			if (i == source_x && source_x > 0) {
				B[i] = A[i];
				continue;
			}
			// get temperature at current position
			value_t tc = A[i];
			// get temperatures of adjacent cells
			value_t tl;
			value_t tr;
			if (i != 0) {
				tl = A[i-1];
			} else if (i == 0 && rank != 0) {
				if(MPI_Recv(&tl, 1, MPI_DOUBLE, rank-1, 42, MPI_COMM_WORLD, MPI_STATUS_IGNORE) != MPI_SUCCESS) {
					fprintf(stderr, "MPI_Recv failed (left) at rank %d.\n", rank);
				}
			} else {
				tl = tc;
			}
			if (i != N-1) {
				tr = A[i+1];
			} else if (i == N-1 && rank != size-1) {
				if(MPI_Recv(&tr, 1, MPI_DOUBLE, rank+1, 42, MPI_COMM_WORLD, MPI_STATUS_IGNORE) != MPI_SUCCESS) {
					fprintf(stderr, "MPI_Recv failed (right) at rank %d.\n", rank);
				}
			} else {
				tr = tc;
			}
			// compute new temperature at current position
			B[i] = tc + 0.2 * (tl + tr + (-2 * tc));
		}
		// swap matrices (just pointers, not content)
		Vector H = A;
		A = B;
		B = H;
		// show intermediate step
		// modification: keep number of lines to print stable
		if (!(t % N)) {
			if(MPI_Gatherv(A, N, MPI_DOUBLE, A, rcv_counts, rcv_displs, MPI_DOUBLE, 0, MPI_COMM_WORLD) != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Gatherv failed at rank %d.\n", rank);
			}
			if (rank == 0) {
				printf("Step t=%d:\t", t);
				printTemperature(A, N_elements);
				printf("\n");
			}
		}
	}
	releaseVector(B);
	// ---------- check ----------
	int success = 1;
	if(MPI_Gatherv(A, N, MPI_DOUBLE, A, rcv_counts, rcv_displs, MPI_DOUBLE, 0, MPI_COMM_WORLD) != MPI_SUCCESS) {
		fprintf(stderr, "MPI_Gatherv failed at rank %d.\n", rank);
	}
	if (rank == 0) {
		printf("Final:\t\t");
		printTemperature(A, N_elements);
		printf("\n");
		for (long long i = 0; i < N_elements; i++) {
			value_t temp = A[i];
			if (273 <= temp && temp <= 273 + 60)
				continue;

			success = 0;
			break;
		}
	printf("Verification: %s\n", (success) ? "OK" : "FAILED");
	value_t cs, min_el, max_el;
	cs = sumVector(A, N_elements, &min_el, &max_el);
	printf("Checksum: %f, (min = %f°C, max = %f°C)\n", cs, min_el-273, max_el-273);
	}
	// ---------- cleanup ----------
	releaseVector(A);
	return success;
}
