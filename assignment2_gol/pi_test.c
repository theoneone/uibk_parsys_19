#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include <math.h>
#include <omp.h>

#define PAGE_SIZE (4096)
#define PAGES_PER_THREAD (1)

static int read_buffer(int fd, void *buffer, size_t size)
{
	ssize_t ret;

	while (size > 0) {
		ret = read(fd, buffer, size);

		if (ret < 0) {
			if (errno == EINTR)
				continue;

			perror("read");
			return -1;
		}

		if (ret == 0) {
			fputs("read: unexpected end-of-file\n", stderr);
			return -1;
		}

		buffer = (char *)buffer + ret;
		size -= ret;
	}

	return 0;
}

static size_t count_circle_hits(const uint16_t *samples, size_t num_samples)
{
	size_t hit_count = 0;
	double x, y;
	size_t i;

	for (i = 0; i < num_samples; ++i) {
		x = ( samples[i]       & 0x0FF) / 256.0;
		y = ((samples[i] >> 8) & 0x0FF) / 256.0;

		if (sqrt(x * x + y * y) <= 1.0)
			hit_count += 1;
	}

	return hit_count;
}

static int do_sampling(int randomfd, uint64_t num_samples, uint64_t *hits)
{
	size_t bufsize, bufcount, roundsz;
	uint64_t local_hitcount = 0;
	uint16_t *buffer;
	int ret;

	bufsize = PAGE_SIZE * PAGES_PER_THREAD;
	bufcount = bufsize / sizeof(buffer[0]);

	buffer = mmap(NULL, bufsize, PROT_READ | PROT_WRITE,
		      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (buffer == MAP_FAILED) {
		perror("mmap");
		return -1;
	}

	while (num_samples > 0) {
#pragma omp critical
		{
			ret = read_buffer(randomfd, buffer, bufsize);
			*hits += local_hitcount;
		}

		if (ret)
			return -1;

		roundsz = num_samples > bufcount ? bufcount : num_samples;
		num_samples -= roundsz;

		local_hitcount = count_circle_hits(buffer, roundsz);
	}

#pragma omp critical
	*hits += local_hitcount;

	munmap(buffer, bufsize);
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

	sample_count /= omp_get_num_threads();

#pragma omp parallel
	if (do_sampling(fd, sample_count, &hits))
		status = EXIT_FAILURE;

	sample_count *= omp_get_num_threads();

	if (status == EXIT_SUCCESS) {
		printf("Hit/sample ratio: %lu,%lu\n", hits, sample_count);
		printf("PI approximation: %f\n", 4.0 * hits / sample_count);
	}

	close(fd);
	return status;
}
