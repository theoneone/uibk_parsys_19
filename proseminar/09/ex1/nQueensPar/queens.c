#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

/****************************** simulation data ******************************/

static unsigned long results_found = 0;
static int n_queens = 8;
static int print_all = 0;
//static int *board;

/************************** command line processing **************************/

static struct option long_opts[] = {
	{ "n-queens", required_argument, NULL, 'n' },
	{ "print-all", no_argument, NULL, 'a'},
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
};

static const char *short_opts = "n:ahV";

static const char *usagestr =
"Usage: queens [OPTIONS...]\n"
"\n"
"Solves the n-queens puzzle and returns the number of solutions.\n"
"\n"
"Possible options:\n"
"\n"
"  --n-queens, -n <number>      Number of queens (default 8)\n"
"  --print-all, -a              Print all found solutions to stdout\n"
"                               as comma separated values.\n"
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
		case 'n':
			n_queens = strtol(optarg, &end, 10);
			break;
		case 'a':
			print_all = 1;
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

	if (n_queens < 1) {
		fputs("Number of queens must be a positive number!\n",
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

static inline void output_to_console(int *board) {
	for (int i = 0; i < n_queens; ++i) {
		printf("%d/%d, ", i, board[i]);
	}
	puts("\n");
}


static inline int iabs(int value)
{
	return value < 0 ? -value : value;
}

static int feasible(int row, int col, int *board) {
	for (int i = 0; i < row; ++i) {
		if(
				// check row: board contains 1 queen by data structure constraints
				col == board[i] || // check column
				iabs(row - i) == iabs(col - board[i]) // check diagonals
				) {
			return 0;
		}
	}
	return 1;
}

// backtracking explained: see https://www.youtube.com/watch?v=R8bM6pxlrLY
static int nqueens(int row, int *board, int n)
{
	int err = 0;
	if(row < n) {
		for (int i = 0; i < n; ++i) {
			if (feasible(row, i, board)) {
				board[row] = i;
#pragma omp parallel sections
				{
//#pragma omp sections
//#pragma omp task default(none) shared(err, n, stderr, print_all) firstprivate(row, board)
#pragma omp section
				{
				int *brd;
				if((brd = (int*)malloc(n*sizeof(int))) == NULL) {
					fputs("Error allocating memory.", stderr);
					err |= 1;
				} else {
					for(int j = 0; j < n; ++j) {
						brd[j] = board[j];
					}
					err |= nqueens(row + 1, brd, n);
					free(brd);
					}
				}
				}
//#pragma omp taskwait
			}
		}
	} else {
#pragma omp critical
		{
		++results_found;
		if (print_all) {
			{
			output_to_console(board);
			}
		}
		}
	}
	return err;
}

int main(int argc, char **argv) {
	int status = EXIT_FAILURE;

	process_options(argc, argv);

//#pragma omp parallel
	{
	int *board;
	if((board = (int*)calloc(n_queens, sizeof(int))) == NULL) {
		fputs("Error allocating memory.", stderr);
	} else {
		nqueens(0, board, n_queens);
		free(board);
		status = EXIT_SUCCESS;
	}
	}
//#pragma omp barrier

	printf("%lu solutions found.\n", results_found);
	return status;
}
