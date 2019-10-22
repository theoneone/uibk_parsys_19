#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

int main(void)
{
	unsigned long total_hit = 0, total_samples = 0;
	unsigned long local_hit, local_samples;
	int status = EXIT_SUCCESS;
	size_t lineno = 1;
	ssize_t ret;

	for (;;) {
		char *line = NULL;
		size_t n = 0;

		errno = 0;
		ret = getline(&line, &n, stdin);
		++lineno;

		if (ret < 0) {
			if (errno != 0) {
				fprintf(stderr, "%zu: getline: %s\n",
					lineno, strerror(errno));
				status = EXIT_FAILURE;
			}
			free(line);
			break;
		}

		if (sscanf(line, "%lu,%lu", &local_hit, &local_samples) != 2) {
			fprintf(stderr,
				"WARNING: skipping unparsable line %zu!\n",
				lineno);
			free(line);
			continue;
		}

		free(line);

		total_hit += local_hit;
		total_samples += local_samples;

		/*if (__builtin_add_overflow(total_hit, local_hit,
					   &total_hit)) {
			fprintf(stderr, "%zu: hit count overflow!\n", lineno);
			status = EXIT_FAILURE;
			break;
		}

		if (__builtin_add_overflow(total_samples, local_samples,
					   &total_samples)) {
			fprintf(stderr, "%zu: sample count overflow!\n",
				lineno);
			status = EXIT_FAILURE;
			break;
		}*/
	}

	if (status == EXIT_SUCCESS) {
		printf("Total hit/sample ratio: %lu / %lu\n",
		       total_hit, total_samples);

		printf("Pi approximation: %f\n",
		       4.0 * (double)total_hit / total_samples);
	}

	return status;
}
