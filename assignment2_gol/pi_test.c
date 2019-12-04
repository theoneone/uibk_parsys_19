#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include <math.h>
#include <omp.h>

static int get_random_seed(int fd, unsigned int *seed)
{
	ssize_t ret;
retry:
	ret = read(fd, seed, sizeof(*seed));

	if (ret < 0) {
		if (errno == EINTR)
			goto retry;

		perror("read");
		return -1;
	}

	if ((size_t)ret < sizeof(*seed)) {
		fputs("read from random device truncated\n", stderr);
		return -1;
	}

	return 0;
}

static int count_circle_hits(int randomfd, uint64_t num_samples,
			     uint64_t *total_hits)
{
	uint64_t i, hit_count = 0;
	unsigned int seed;
	double x, y;
	int ret;

#pragma omp critical
	ret = get_random_seed(randomfd, &seed);

	if (ret)
		return -1;

	for (i = 0; i < num_samples; ++i) {
		x = (double)rand_r(&seed) / RAND_MAX;
		y = (double)rand_r(&seed) / RAND_MAX;

		if (sqrt(x * x + y * y) <= 1.0)
			hit_count += 1;
	}

#pragma omp atomic
	*total_hits += hit_count;

	return 0;
}

int main(int argc, char **argv)
{
	uint64_t hits = 0, sample_count = 0;
	int fd, status = EXIT_SUCCESS;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <sample count>\n", argv[0]);
		return EXIT_FAILURE;
	}

	errno = 0;
	sample_count = strtoul(argv[1], NULL, 10);

	if (sample_count == ULONG_MAX && errno != 0) {
		perror("parsing sample count from command line");
		return EXIT_FAILURE;
	}

	fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0) {
		perror("/dev/urandom");
		return EXIT_FAILURE;
	}

#pragma omp parallel
	if (count_circle_hits(fd, sample_count / omp_get_num_threads(), &hits))
		status = EXIT_FAILURE;

	if (status == EXIT_SUCCESS) {
		printf("Hit/sample ratio: %lu,%lu\n", hits, sample_count);
		printf("PI approximation: %f\n", 4.0 * hits / sample_count);
	}

	close(fd);
	return status;
}
