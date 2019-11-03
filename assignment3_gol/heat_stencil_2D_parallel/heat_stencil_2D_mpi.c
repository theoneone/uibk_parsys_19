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

value_t sumVector(Vector V, size_t N, value_t *min, value_t *max);

int calculate_part(size_t size, size_t rank, size_t M, size_t N_elements, size_t T);

// -- simulation code ---

int main(int argc, char **argv) {
	int size, rank;
	size_t M, N, T;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size); // get the number of ranks
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); // get the rank of the caller

	// 'parsing' optional input parameter = problem size
	M = 1;
	N = 2000;
	if (argc > 1) {
		N = strtoul(argv[1], NULL, 10);
	}
	if (argc > 2) {
		M = strtoul(argv[2], NULL, 10);
	}
	T = N * 500;

	int success = 1;
	if (rank == 0) {
		fprintf(stderr, "Computing heat-distribution for room size N=%lu for T=%lu timesteps\n", N, T);
	}
	fprintf(stderr, "Starting computation on rank %d of %d\n", rank, size);
	success = calculate_part(size, rank, M, N, T);

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

value_t sumVector(Vector V, size_t N, value_t *min, value_t *max) {
	value_t result = 0.0;
	if (min != NULL) *min = V[0];
	if (max != NULL) *max = V[0];

	for (size_t i = 0; i < N; ++i) {
		result += V[i];
		if(min != NULL && V[i] < *min) *min = V[i];
		if(max != NULL && V[i] > *max) *max = V[i];
	}
	return result;
}

int calculate_part(size_t size, size_t rank, size_t M, size_t N_elements, size_t T) {
	// ---------- setup ----------

	size_t frame_ID = 0;
	size_t N = (N_elements + size-1) / size;
	IVector rcv_counts, rcv_displs;
	rcv_counts = createIVector(size);
	rcv_displs = createIVector(size);
	size_t MM = M + 2;
	int displ = MM;
	for (size_t i = 0; i < size; ++i) {
		rcv_counts[i] = N * MM;
		rcv_displs[i] = displ;
		displ += N * MM;
	}
	fprintf(stderr, "rank: %lu, N = %lu, N_el = %lu, last=%lu\n", rank, N, N_elements, N_elements % N);//XXX
	size_t NN = (N_elements % N == 0 ? N : N_elements % N);
	rcv_counts[size-1] = NN * MM;
	if (rank == size-1) {
		N = NN; // last rank might be not entirely filled, remove unnecessary elements
	}
	fprintf(stderr, "cnt: ");//XXX 7 Zeilen
	for(size_t i=0; i<size;++i) fprintf(stderr, "%d, ", rcv_counts[i]);
	fprintf(stderr, "\n");
	fprintf(stderr, "dsp: ");
	for(size_t i=0; i<size;++i) fprintf(stderr, "%d, ", rcv_displs[i]);
	fprintf(stderr, " %d\n", rcv_counts[size-1]+rcv_displs[size-1]);
	fprintf(stderr, "rank %lu, N=%lu\n", rank, N);

	// create a buffer A for storing temperature fields
	// create a second buffer B for the computation
	Vector A, B;
	if (rank == 0) {
		A = createVector(M + 2, N_elements + 2);
		B = createVector(M + 2, N_elements + 2);
	} else {
		A = createVector(M + 2, N + 2);
		B = createVector(M + 2, N + 2);
	}
	NN = N + 2;

	// set up initial conditions in A
	for (size_t j = 1; j <= M; ++j) {
		for (size_t i = 1; i <= N; ++i) {
			A[i + NN * j] = 273; // temperature is 0° C everywhere (273 K)
		}
	}
	// and there is a heat source in one corner
	size_t source_x = 0; // zero value: source not in this rank
	size_t source_y = M / 4 + 1;
	if ((unsigned)rcv_displs[rank] <= N_elements / 4 && (unsigned)rcv_displs[rank] + N > N_elements / 4) {
		source_x = (N_elements / 4) % N + 1;
		A[source_x + NN * source_y] = 273 + 60;
		fprintf(stderr, "rank %lu, seed at %lu/%lu, heat = %f\n", rank, source_x, source_y, A[source_x + NN * source_y]);//XXX
	}
	fprintf(stderr, "rank %lu, seed at %lu/%lu (find source array)\n", rank, source_x, source_y);//XXX
	if (rank == 0) {
		gen_image(STDOUT_FILENO, frame_ID++, A, M + 2, N_elements + 2, 273, 273 + 60);
		printTemperature(A, (M+2)*(N_elements+2));
	}
	// ---------- compute ----------
	// for each time step ..
	for (size_t t = 0; t < T; t++) {
		if (rank != 0) {
			if(MPI_Ssend(&(A[NN]), M, MPI_DOUBLE, rank-1, 42, MPI_COMM_WORLD) != MPI_SUCCESS) {//FIXME
				fprintf(stderr, "MPI_Send failed (left) at rank %lu.\n", rank);
			}
			if(MPI_Recv(A, M, MPI_DOUBLE, rank-1, 42, MPI_COMM_WORLD, MPI_STATUS_IGNORE) != MPI_SUCCESS) {//r_tl
				fprintf(stderr, "MPI_Recv failed (left) at rank %lu.\n", rank);
			}
		} else {
			for (size_t j = 0; j < NN; ++j) {
				A[j] = A[NN+j];
			}
		}
		if (rank != size-1) {
			if(MPI_Recv(&(A[(M+1)*(N+1)]), M, MPI_DOUBLE, rank+1, 42, MPI_COMM_WORLD, MPI_STATUS_IGNORE) != MPI_SUCCESS) {//r_tr
				fprintf(stderr, "MPI_Recv failed (right) at rank %lu.\n", rank);
			}
			if(MPI_Ssend(&(A[(M+1)*N]), M, MPI_DOUBLE, rank+1, 42, MPI_COMM_WORLD) != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Send failed (right) at rank %lu.\n", rank);
			}
		} else {
			for (size_t j = 0; j < N; ++j) {
				A[(M+1)*NN + j] = A[(M+1)*(NN-1) + j];
			}
		}

		// .. we propagate the temperature
		for (size_t j = 1; j <= M; ++j) {
			for (size_t i = 1; i <= N; ++i) {
				// center stays constant (the heat is still on)
				if (i == source_x && source_x > 0 && j == source_y) {
					B[i + NN * j] = A[i + NN * j];
					continue;
				}
				// compute new temperature at current position
				B[i + NN * j] = A[i + NN * j] +
						0.2 * (A[i-1 + NN*j] + A[i+1 + NN*j] + A[i + (NN-1)*j] + A[i + (NN+1)*j] + (-4 * A[i + NN*j]));
			}
		}
		// swap matrices (just pointers, not content)
		Vector H = A;
		A = B;
		B = H;
		// show intermediate step
		// modification: keep number of lines to print stable
		if (!(t % N)) {
			if(MPI_Gatherv(A, N * MM, MPI_DOUBLE, A, rcv_counts, rcv_displs, MPI_DOUBLE, 0, MPI_COMM_WORLD) != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Gatherv failed at rank %lu.\n", rank);
			}
			if (rank == 0) {
				gen_image(STDOUT_FILENO, frame_ID++, A, M + 2, N_elements + 2, 273, 273 + 60);
				printTemperature(A, (M+2)*(N_elements+2));
			}
		}
	}
	releaseVector(B);
	// ---------- check ----------
	int success = 1;
	if(MPI_Gatherv(A, N * MM, MPI_DOUBLE, A, rcv_counts, rcv_displs, MPI_DOUBLE, 0, MPI_COMM_WORLD) != MPI_SUCCESS) {
		fprintf(stderr, "MPI_Gatherv failed at rank %lu.\n", rank);
	}
	if (rank == 0) {
		gen_image(STDOUT_FILENO, frame_ID++, A, M + 2, N_elements + 2, 273, 273 + 60);
		printTemperature(A, (M+2)*(N_elements+2));
		for (size_t i = 0; i <= (M+2)*(N_elements+1); ++i) {
			value_t temp = A[i];
			if (273 <= temp && temp <= 273 + 60)
				continue;

			success = 0;
			break;
		}
	fprintf(stderr, "Verification: %s\n", (success) ? "OK" : "FAILED");
	value_t cs, min_el, max_el;
	cs = sumVector(A, (M+2)*(N_elements+2), &min_el, &max_el);
	fprintf(stderr, "Checksum: %f, (min = %f°C, max = %f°C)\n", cs, min_el-273, max_el-273);
	}
	// ---------- cleanup ----------
	releaseVector(A);
	return success;
}
