#include <mpi.h>
#include "../heat_stencil.h"

// -- vector utilities --

typedef value_t *Vector;
typedef int *IVector;

Vector createVector(int N);

void releaseVector(Vector m);

IVector createIVector(int N);

void releaseIVector(IVector m);

value_t sumVector(Vector V, int N, value_t *min, value_t *max);

int calculate_part(int size, int rank, int N, int T);

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
	size_t frame_ID = 0;
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
	rcv_counts[size-1] = N_elements % N == 0 ? N : N_elements % N;
	if (rank == size-1) {
		N = rcv_counts[rank]; // last rank might be not entirely filled, remove unnecessary elements
	}

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
	}
	if (rank == 0) {
		gen_image(STDOUT_FILENO, frame_ID++, A, N, 1, 273, 273 + 60);
	}
	// ---------- compute ----------
	// for each time step ..
	for (int t = 0; t < T; t++) {
		value_t r_tl, r_tr;
		if (rank != 0) {
			if(MPI_Ssend(&(A[0]), 1, MPI_DOUBLE, rank-1, 42, MPI_COMM_WORLD) != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Send failed (left) at rank %d.\n", rank);
			}
			if(MPI_Recv(&r_tl, 1, MPI_DOUBLE, rank-1, 42, MPI_COMM_WORLD, MPI_STATUS_IGNORE) != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Recv failed (left) at rank %d.\n", rank);
			}
		}
		if (rank != size-1) {
			if(MPI_Recv(&r_tr, 1, MPI_DOUBLE, rank+1, 42, MPI_COMM_WORLD, MPI_STATUS_IGNORE) != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Recv failed (right) at rank %d.\n", rank);
			}
			if(MPI_Ssend(&(A[N-1]), 1, MPI_DOUBLE, rank+1, 42, MPI_COMM_WORLD) != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Send failed (right) at rank %d.\n", rank);
			}
		}
		if (rank != 0) {
		}
		// .. we propagate the temperature
		for (long long i = 0; i < N; i++) {
			// center stays constant (the heat is still on)
			if (i == source_x && source_x >= 0) {
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
				tl = r_tl;
			} else {
				tl = tc;
			}
			if (i != N-1) {
				tr = A[i+1];
			} else if (i == N-1 && rank != size-1) {
				tr = r_tr;
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
				gen_image(STDOUT_FILENO, frame_ID++, A, N, 1, 273, 273 + 60);
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
		gen_image(STDOUT_FILENO, frame_ID++, A, N, 1, 273, 273 + 60);
		for (long long i = 0; i < N_elements; i++) {
			value_t temp = A[i];
			if (273 <= temp && temp <= 273 + 60)
				continue;

			success = 0;
			break;
		}
	fprintf(stderr, "Verification: %s\n", (success) ? "OK" : "FAILED");
	value_t cs, min_el, max_el;
	cs = sumVector(A, N_elements, &min_el, &max_el);
	fprintf(stderr, "Checksum: %f, (min = %f°C, max = %f°C)\n", cs, min_el-273, max_el-273);
	}
	// ---------- cleanup ----------
	releaseVector(A);
	return success;
}
