#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include <math.h>

static int get_sample(int fd, uint8_t *sample)
{
	int ret;
retry:
	ret = read(fd, sample, sizeof(*sample));

	if (ret < 0) {
		if (errno == EINTR)
			goto retry;

		perror("reading random sample");
		return -1;
	}

	if (ret == 0) {
		fputs("reading random sample: unexpected end-of-file\n",
		      stderr);
		return -1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	uint64_t i, hit = 0, sample_count = 0;
	int status = EXIT_FAILURE;
	uint8_t x, y;
	int fd;

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

	fd = open("/dev/random", O_RDONLY);
	if (fd < 0) {
		perror("/dev/random");
		return EXIT_FAILURE;
	}

	for (i = 0; i < sample_count; ++i) {
		if (get_sample(fd, &x))
			goto out;
		if (get_sample(fd, &y))
			goto out;

		if (sqrt(x * x + y * y) <= 256.0)
			hit += 1;
	}

	printf("%lu,%lu\n", hit, sample_count);

	status = EXIT_SUCCESS;
out:
	close(fd);
	return status;
}
