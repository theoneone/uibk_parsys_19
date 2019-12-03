#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <time.h>

/****************************** matrix typedef *******************************/
typedef struct {
	int cols;
	int rows;
	double *data;
} matrix;

/****************************** simulation data ******************************/

static int a_columns = 1000;
static int a_rows = 1000;
static int b_columns = 1000;
static double min_value = 10.0;
static double max_value = 100.0;
static unsigned int rand_seed = 0;
static int get_result = 0;
static int get_inputs = 0;
static int to_integer = 0;

/************************** command line processing **************************/

static struct option long_opts[] = {
	{ "a-columns", required_argument, NULL, 'c' },
	{ "a-rows", required_argument, NULL, 'r' },
	{ "b-columns", required_argument, NULL, 'b' },
	{ "min-value", required_argument, NULL, 'm' },
	{ "max-value", required_argument, NULL, 'M' },
	{ "rand-seed", optional_argument, NULL, 's' },
	{ "get-result", no_argument, NULL, 'o'},
	{ "get-inputs", no_argument, NULL, 'i'},
	{ "to-integer", no_argument, NULL, 't'},
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
};

static const char *short_opts = "c:r:b:m:M:s::oithV";

static const char *usagestr =
"Usage: matrix_mul [OPTIONS...]\n"
"\n"
"Runs a matrix multiplication A*B=C with randomized input.\n"
"\n"
"Possible options:\n"
"\n"
"  --a-columns, -c <number>     Number of columns of input matrix A\n"
"  --a-rows, -r <number>        Number of rows of input matrix A\n"
"  --b-columns, -b <number>     Number of columns of input matrix B\n"
"                               The number of rows of B equals to -l\n"
"  --min-value, -m <number>     Lower threshold for randomized\n"
"                               input values.\n"
"  --max-value, -M <number>     Upper threshold for randomized\n"
"                               input values.\n"
"  --rand-seed[=num], -s[num]   Integer seed for random generator.\n"
"                               Set parameter without value for randomized seed.\n"
"  --get-result, -o             Prompt the result to stdout\n"
"                               as comma separated values.\n"
"  --get-inputs, -i             Prompt the generated input matrices to stdout\n"
"                               as comma separated values.\n"
"  --to-integer, -t             Round input data."
"\n"
"  --help, -h                   Display this help text and exit.\n"
"  --version, -V                Display version information and exit.\n"
"\n";

#define GPL_URL "https://gnu.org/licenses/gpl.html"

static const char *versionstr =
"matrix_mul (the serial one, naive implementation with for loops) 0.1\n"
"Copyright (C) 2019 David Oberhollenzer, Gerhard Aigner\n"
"License GPLv3+: GNU GPL version 3 or later <" GPL_URL ">.\n"
"This is free software: you are free to change and redistribute it."
"There is NO WARRANTY, to the extent permitted by law."
"\n";

static void process_options(int argc, char **argv)
{
	char *end;
	int i;

	for (;;) {
		i = getopt_long(argc, argv, short_opts, long_opts, NULL);
		if (i == -1)
			break;

		end = NULL;
		errno = 0;

		switch (i) {
		case 'c':
			a_columns = strtol(optarg, &end, 10);
			break;
		case 'r':
			a_rows = strtol(optarg, &end, 10);
			break;
		case 'b':
			b_columns = strtol(optarg, &end, 10);
			break;
		case 'm':
			min_value = strtod(optarg, &end);
			break;
		case 'M':
			max_value = strtod(optarg, &end);
			break;
		case 's':
			if (optarg != NULL) {
				rand_seed = strtol(optarg, &end, 10);
			} else {
				rand_seed = time(NULL);
			}
			break;
		case 'o':
			get_result = 1;
			break;
		case 'i':
			get_inputs = 1;
			break;
		case 't':
			to_integer = 1;
			break;
		case 'h':
			fputs(usagestr, stdout);
			exit(EXIT_SUCCESS);
		case 'V':
			fputs(versionstr, stdout);
			exit(EXIT_SUCCESS);
		default:
			goto fail_arg;
		}

		if (end != NULL && (*end != '\0' || errno != 0)) {
			fprintf(stderr, "Error parsing argument "
				"for option `-%c`.\n", i);
			goto fail_arg;
		}
	}

	if (a_columns < 1 || a_rows < 1 || b_columns < 1) {
		fputs("Matrix dimensions must be positive numbers!\n",
		      stderr);
		goto fail_arg;
	}

	if (min_value > max_value) {
		fputs("Minimum value needs to be less than or equal to maximum value!\n",
		      stderr);
		goto fail_arg;
	}

	if (optind < argc) {
		fputs("Unknown extra arguments.\n", stderr);
		goto fail_arg;
	}

	return;
fail_arg:
	fputs("Try `matrix_mul --help' for more information.\n", stderr);
	exit(EXIT_FAILURE);
}

/*****************************************************************************/

static double gen_number(const double min, const double max)
{
	return min + ((double)rand() / RAND_MAX) * (max - min);
}

static double gen_integer(const double min, const double max)
{
	return round(min + ((double)rand() / RAND_MAX) * (max - min));
}

static int init_matrix(matrix *a, const int columns, const int rows, double (*generator)(const double, const double))
{
	a->data = (double*)malloc(sizeof(double) * columns * rows);
	if (a->data == NULL) {
		perror("allocating matrix");
		return -1;
	}
	a->cols = columns;
	a->rows = rows;

	if (generator != NULL) {
		for (int i = 0; i < columns * rows; ++i) {
			a->data[i] = generator(min_value, max_value);
		}
	}
	return 0;
}

static inline int index(const int col, const int row, const int columns)
{
	return row*columns + col;
}

static void multiply(const matrix a, const matrix b, matrix c)
{
	double tmp;
	for (int j = 0; j < c.rows; ++j) {
		for (int i = 0; i < c.cols; ++i) {
			tmp = 0;
			for (int k = 0; k < a.cols; ++k) {
				tmp += a.data[index(k, j, a.cols)] * b.data[index(i, k, b.cols)];
			}
			c.data[index(i, j, c.cols)] = tmp;
		}
	}
}

static int output_to_console(const matrix c)
{
	for (int j = 0; j < c.rows; ++j) {
		for (int i = 0; i < c.cols; ++i) {
			fprintf(stdout, "%f,", c.data[index(i, j, c.cols)]);
		}
		fprintf(stdout, "\n");
	}
	return 0;
}

int main(int argc, char **argv)
{
	int status = EXIT_FAILURE;


	process_options(argc, argv);

	matrix a = {.cols = 0, .rows = 0, .data = NULL};
	matrix b = {.cols = 0, .rows = 0, .data = NULL};
	matrix c = {.cols = 0, .rows = 0, .data = NULL};

	double (* gen_num)(const double, const double) = to_integer ? &gen_integer : &gen_number;
	if (init_matrix(&a, a_columns, a_rows, gen_num)) goto out;
	if (init_matrix(&b, b_columns, a_columns, gen_num)) goto out;
	if (init_matrix(&c, b_columns, a_rows, NULL)) goto out;

	time_t t_start, t_end;
	time(&t_start);
	multiply(a, b, c);
	time(&t_end);
	fprintf(!get_inputs && !get_result ? stdout : stderr, "Used time for multiplication: %f\n", difftime(t_end, t_start));
	if(get_inputs) {
		output_to_console(a);
		puts("\n");
		output_to_console(b);
		puts("\n");
	}
	if(get_result) {
		output_to_console(c);
	}

	status = EXIT_SUCCESS;
out:
	if(a.data != NULL) free(a.data);
	if(b.data != NULL) free(b.data);
	if(c.data != NULL) free(c.data);
	return status;
}
