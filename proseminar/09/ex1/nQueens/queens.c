#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#define MAX_QUEENS 32

typedef struct {
	uint8_t columns[MAX_QUEENS];
} __attribute__ ((aligned (64))) board_t;

static inline int get_queen_col_from_row(const board_t *board, unsigned int row)
{
	return board->columns[row];
}

static inline void set_queen(board_t *board, unsigned int row, unsigned int col)
{
	board->columns[row] = col;
}

/****************************** simulation data ******************************/

static unsigned long results_found = 0;
static int n_queens = 8;
static int print_all = 0;

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

	if (n_queens > MAX_QUEENS) {
		fprintf(stderr, "Number of queens must be at most %d!\n",
			MAX_QUEENS);
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

static inline void output_to_console(const board_t *board)
{
	for (int i = 0; i < n_queens; ++i)
		printf("%d/%d, ", i, get_queen_col_from_row(board, i));

	puts("\n");
}

static inline int iabs(int value)
{
	return value < 0 ? -value : value;
}

static int feasible(const board_t *board, int row, int col) {
	for (int i = 0; i < row; ++i) {
		int pcol = get_queen_col_from_row(board, i);

		if(
				// check row: board contains 1 queen by data structure constraints
				col == pcol || // check column
				iabs(row - i) == iabs(col - pcol) // check diagonals
				) {
			return 0;
		}
	}
	return 1;
}

// backtracking explained: see https://www.youtube.com/watch?v=R8bM6pxlrLY
static void nqueens(board_t board, int row)
{
	if(row < n_queens) {
		for (int i = 0; i < n_queens; ++i) {
			if (feasible(&board, row, i)) {
				set_queen(&board, row, i);
				nqueens(board, row + 1);
			}
		}
	} else {
		++results_found;
		if (print_all) {
			output_to_console(&board);
		}
	}
}

int main(int argc, char **argv)
{
	board_t empty;

	process_options(argc, argv);

	memset(&empty, 0, sizeof(empty));
	nqueens(empty, 0);

	printf("%lu solutions found.\n", results_found);
	return EXIT_SUCCESS;
}
