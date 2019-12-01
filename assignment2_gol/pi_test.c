#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include <math.h>

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
	uint8_t x, y;
	size_t i;

	for (i = 0; i < num_samples; ++i) {
		x =  samples[i]       & 0x0FF;
		y = (samples[i] >> 8) & 0x0FF;

		if (sqrt(x * x + y * y) <= 256.0)
			hit_count += 1;
	}

	return hit_count;
}

static int do_sampling(int randomfd, uint64_t num_samples, uint64_t *hits,
		       size_t num_threads)
{
	size_t i, hitcount[num_threads];
	uint16_t *buffers[num_threads];
	uint64_t maxroundsz, roundsz;
	int ret = -1;

	for (i = 0; i < num_threads; ++i) {
		buffers[i] = mmap(NULL, PAGE_SIZE * PAGES_PER_THREAD,
				  PROT_READ | PROT_WRITE,
				  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

		if (buffers[i] == MAP_FAILED) {
			perror("mmap");
			num_threads = i;
			goto out;
		}
	}

	maxroundsz = (PAGE_SIZE * PAGES_PER_THREAD) / sizeof(buffers[0][0]);
	*hits = 0;

	while (num_samples > 0) {
		roundsz = num_samples / num_threads;

		if (roundsz > maxroundsz)
			roundsz = maxroundsz;

		num_samples -= roundsz * num_threads;

		for (i = 0; i < num_threads; ++i) {
			ret = read_buffer(randomfd, buffers[i],
					  roundsz * sizeof(buffers[0][0]));
			if (ret)
				return -1;
		}

		for (i = 0; i < num_threads; ++i) {
			hitcount[i] = count_circle_hits(buffers[i], roundsz);
		}

		for (i = 0; i < num_threads; ++i)
			(*hits) += hitcount[i];
	}

	ret = 0;
out:
	for (i = 0; i < num_threads; ++i)
		munmap(buffers[i], PAGE_SIZE * PAGES_PER_THREAD);

	return ret;
}

int main(int argc, char **argv)
{
	uint64_t hits = 0, sample_count = 0;
	int fd, status = EXIT_FAILURE;

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

	if (do_sampling(fd, sample_count, &hits, 1))
		goto out;

	printf("Hit/sample ratio: %lu,%lu\n", hits, sample_count);
	printf("PI approximation: %f\n", 4.0 * hits / sample_count);

	status = EXIT_SUCCESS;
out:
	close(fd);
	return status;
}
