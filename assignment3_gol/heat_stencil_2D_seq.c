#include "heat_stencil.h"

int main(int argc, char **argv)
{
	size_t x, y, t, N, T, frame_ID = 0;
	size_t source_x, source_y;
	value_t *A, *B, *H;

	N = 2000;

	if (argc > 1)
		N = strtoul(argv[1], NULL, 10);

	T = N * 500;

	source_x = N / 4;
	source_y = N / 4;

	// ---------- setup ----------
	A = calloc(sizeof(value_t), N * N);
	B = calloc(sizeof(value_t), N * N);

	// initial temperature is 0C everywhere (273 K)
	for (y = 0; y < N; ++y) {
		for (x = 0; x < N; ++x) {
			A[y * N + x] = 273;
		}
	}

	A[source_y * N + source_x] = 273 + 60;

	// ---------- compute ----------
	for (t = 0; t < T; ++t) {
		if (t % 1000 == 0) {
			gen_image(STDOUT_FILENO, frame_ID++,
				  A, N, N, 273, 273 + 60);
		}

		for (y = 0; y < N; ++y) {
			for (x = 0; x < N; ++x) {
				if (x == source_x && y == source_y) {
					B[y * N + x] = A[y * N + x];
				} else {
					value_t tc = A[y * N + x];

					value_t tl = (x > 0) ? A[y * N + x - 1] : tc;
					value_t tr = (x < N - 1) ? A[y * N + x	+ 1] : tc;
					value_t to = (y > 0) ? A[(y - 1) * N + x] : tc;
					value_t tu = (y < N - 1) ? A[(y + 1) * N + x] : tc;

					B[y * N + x] = tc + 0.2 * (tl + tr + to + tu + (-4 * tc));
				}
			}
		}

		H = A;
		A = B;
		B = H;
	}

// added image for final result
	gen_image(STDOUT_FILENO, frame_ID,
		  A, N, N, 273, 273 + 60);

	free(B);
	free(A);
	return EXIT_SUCCESS;
}
