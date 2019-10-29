#include <mpi.h>
#include "../heat_stencil.h"

// -- vector utilities --

typedef value_t *Vector;
typedef int *IVector;

Vector createVector(size_t M, size_t N);

void releaseVector(Vector m);

IVector createIVector(size_t N);

void releaseIVector(IVector m);

void printTemperature(Vector m, int N);

value_t sumVector(Vector V, int N, value_t *min, value_t *max);

int calculate_part(size_t size, size_t rank, size_t N, size_t T);

// -- simulation code ---

int main(int argc, char **argv) {
	int size, rank;
	size_t N, T;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size); // get the number of ranks
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); // get the rank of the caller

	// 'parsing' optional input parameter = problem size
	N = 2000;
	if (argc > 1) {
		N = strtoul(argv[1], NULL, 10);
	}
	T = N * 500;

	int success = 1;
	if (rank == 0) {
		fprintf(stderr, "Computing heat-distribution for room size N=%lu for T=%lu timesteps\n", N, T);
	}
	success = calculate_part(size, rank, N, T);

	MPI_Finalize();
	// done
	return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}

Vector createVector(size_t M, size_t N) {
	// create data and index vector
	return malloc(sizeof(value_t) * M * N);
}

void releaseVector(Vector m) { free(m); }

IVector createIVector(size_t N) {
	// create data and index vector
	return malloc(sizeof(size_t) * N);
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
	fprintf(stderr, "X");
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
		fprintf(stderr, "%c", colors[c]);
	}
	// right wall
	fprintf(stderr, "X\n");
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

int calculate_part(size_t size, size_t rank, size_t N_elements, size_t T) {
	// ---------- setup ----------
	size_t frame_ID = 0;
	size_t N = (N_elements + size-1) / size;
	IVector rcv_counts, rcv_displs;
	rcv_counts = createIVector(size);
	rcv_displs = createIVector(size);
	int displ = 0;
	for (size_t i = 0; i < size; ++i) {
		rcv_counts[i] = N;
		rcv_displs[i] = displ;
		displ += N;
	}
	fprintf(stderr, "rank: %d, N = %d, N_el = %d, last=%d\n", rank, N, N_elements, N_elements % N);//XXX
	rcv_counts[size-1] = N_elements % N == 0 ? N : N_elements % N;
	if (rank == size-1) {
		N = rcv_counts[rank]; // last rank might be not entirely filled, remove unnecessary elements
	}
	fprintf(stderr, "cnt: ");//XXX 7 Zeilen
	for(int i=0; i<size;++i) fprintf(stderr, "%d, ", rcv_counts[i]);
	fprintf(stderr, "\n");
	fprintf(stderr, "dsp: ");
	for(int i=0; i<size;++i) fprintf(stderr, "%d, ", rcv_displs[i]);
	fprintf(stderr, " %d\n", rcv_counts[size-1]+rcv_displs[size-1]);
	fprintf(stderr, "rank %d, N=%d\n", rank, N);

	// create a buffer A for storing temperature fields
	// create a second buffer B for the computation
	Vector A, B;
	int NN; // length of A (N is length considered for calculation)
	if (rank == 0) {
		A = createVector(1, N_elements + 2);
		B = createVector(1, N_elements + 2);
		NN = N_elements;
	} else {
		A = createVector(1, N + 2);
		B = createVector(1, N + 2);
		NN = N;
	}
	// set up initial conditions in A
	for (int i = 1; i <= NN; i++) {
		A[i] = 273; // temperature is 0° C everywhere (273 K)
	}
	// and there is a heat source in one corner
	long source_x = -1; // negative value: source not in this rank
	if (rcv_displs[rank] <= N_elements / 4 && rcv_displs[rank] + N > N_elements / 4) {
		source_x = (N_elements / 4) % N + 1;
		A[source_x] = 273 + 60;
		fprintf(stderr, "rank %d, seed at %d, heat = %f\n", rank, source_x, A[source_x]);//XXX
	}
	fprintf(stderr, "rank %d, seed at %d (find source array)\n", rank, source_x);//XXX
	if (rank == 0) {
		gen_image(STDOUT_FILENO, frame_ID++, &(A[1]), N_elements, 1, 273, 273 + 60);
		printTemperature(&(A[1]), N_elements);
	}
	// ---------- compute ----------
	// for each time step ..
	for (size_t t = 0; t < T; t++) {
		if (rank != 0) {
			if(MPI_Ssend(&(A[1]), 1, MPI_DOUBLE, rank-1, 42, MPI_COMM_WORLD) != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Send failed (left) at rank %lu.\n", rank);
			}
			if(MPI_Recv(&(A[0]), 1, MPI_DOUBLE, rank-1, 42, MPI_COMM_WORLD, MPI_STATUS_IGNORE) != MPI_SUCCESS) {//r_tl
				fprintf(stderr, "MPI_Recv failed (left) at rank %lu.\n", rank);
			}
		} else {
			A[0] = A[1];
		}
		if (rank != size-1) {
			if(MPI_Recv(&(A[N+1]), 1, MPI_DOUBLE, rank+1, 42, MPI_COMM_WORLD, MPI_STATUS_IGNORE) != MPI_SUCCESS) {//r_tr
				fprintf(stderr, "MPI_Recv failed (right) at rank %lu.\n", rank);
			}
			if(MPI_Ssend(&(A[N]), 1, MPI_DOUBLE, rank+1, 42, MPI_COMM_WORLD) != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Send failed (right) at rank %lu.\n", rank);
			}
		} else {
			A[N+1] = A[N];
		}

		// .. we propagate the temperature
		for (size_t i = 1; i <= N; i++) {
			// center stays constant (the heat is still on)
			if (i == source_x && source_x >= 0) {
				B[i] = A[i];
				if (!(t % N) && rank == 0) {//XXX
					fprintf(stderr, "src_x=%ld, val=%f\n", i, A[i]);
				}
				continue;
			}
			// compute new temperature at current position
			B[i] = A[i] + 0.2 * (A[i-1] + A[i+1] + (-2 * A[i]));
		}
		// swap matrices (just pointers, not content)
		Vector H = A;
		A = B;
		B = H;
		// show intermediate step
		// modification: keep number of lines to print stable
		if (!(t % N)) {
			if(MPI_Gatherv(&(A[1]), N, MPI_DOUBLE, &(A[1]), rcv_counts, rcv_displs, MPI_DOUBLE, 0, MPI_COMM_WORLD) != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Gatherv failed at rank %lu.\n", rank);
			}
			if (rank == 0) {
				gen_image(STDOUT_FILENO, frame_ID++, &(A[1]), N_elements, 1, 273, 273 + 60);
				printTemperature(&(A[1]), N_elements);
			}
		}
	}
	releaseVector(B);
	// ---------- check ----------
	int success = 1;
	if(MPI_Gatherv(&(A[1]), N, MPI_DOUBLE, &(A[1]), rcv_counts, rcv_displs, MPI_DOUBLE, 0, MPI_COMM_WORLD) != MPI_SUCCESS) {
		fprintf(stderr, "MPI_Gatherv failed at rank %lu.\n", rank);
	}
	if (rank == 0) {
		gen_image(STDOUT_FILENO, frame_ID++, &(A[1]), N_elements, 1, 273, 273 + 60);
		printTemperature(&(A[1]), N_elements);
		for (size_t i = 1; i <= N_elements; i++) {
			value_t temp = A[i];
			if (273 <= temp && temp <= 273 + 60)
				continue;

			success = 0;
			break;
		}
	fprintf(stderr, "Verification: %s\n", (success) ? "OK" : "FAILED");
	value_t cs, min_el, max_el;
	cs = sumVector(&(A[1]), N_elements, &min_el, &max_el);
	fprintf(stderr, "Checksum: %f, (min = %f°C, max = %f°C)\n", cs, min_el-273, max_el-273);
	}
	// ---------- cleanup ----------
	releaseVector(A);
	return success;
}
