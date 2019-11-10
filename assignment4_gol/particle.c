#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <time.h>

#include "particle.h"

/****************************** simulation data ******************************/

static double width = 1000.0;
static double height = 1000.0;
static double min_mass = 10.0;
static double max_mass = 100.0;
static double dt = 1.0;
static size_t count = 1000;
static size_t steps = 1000;
static double G = 1.0;//6.67430e-11;

static particle_t *particles = NULL;

/************************** command line processing **************************/

static struct option long_opts[] = {
	{ "field-width", required_argument, NULL, 'x' },
	{ "field-height", required_argument, NULL, 'y' },
	{ "count", required_argument, NULL, 'c' },
	{ "min-mass", required_argument, NULL, 'm' },
	{ "max-mass", required_argument, NULL, 'M' },
	{ "steps", required_argument, NULL, 's' },
	{ "time", required_argument, NULL, 't' },
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
};

static const char *short_opts = "x:y:c:m:M:s:t:hV";

static const char *usagestr =
"Usage: particle [OPTIONS...]\n"
"\n"
"Runs a simple 2D particle simulation.\n"
"\n"
"WARNING: This craps a lot of data into the current working directory!\n"
"\n"
"Possible options:\n"
"\n"
"  --field-width, -x <number>   The width of the field in which to spawn\n"
"                               particles.\n"
"  --field-height, -y <number>  The height of the field in which to spawn\n"
"                               particles.\n"
"  --count, -c <number>         How many particles to spawn.\n"
"  --min-mass, -m <number>      Lower threshold for randomized\n"
"                               particle mass.\n"
"  --max-mass, -M <number>      Uppser threshold for randomized\n"
"                               particle mass.\n"
"  --steps, -s <number>         How many simulation steps to compute.\n"
"  --time, -t <number>          Time difference for each step.\n"
"\n"
"  --help, -h                   Display this help text and exit.\n"
"  --version, -V                Display version information and exit.\n"
"\n";

#define GPL_URL "https://gnu.org/licenses/gpl.html"

static const char *versionstr =
"particle (the serial one) 0.1\n"
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
		case 'x':
			width = strtod(optarg, &end);
			break;
		case 'y':
			height = strtod(optarg, &end);
			break;
		case 'c':
			count = strtol(optarg, &end, 10);
			break;
		case 'm':
			min_mass = strtod(optarg, &end);
			break;
		case 'M':
			max_mass = strtod(optarg, &end);
			break;
		case 's':
			steps = strtol(optarg, &end, 10);
			break;
		case 't':
			dt = strtod(optarg, &end);
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

	if (count < 1 || steps < 1 || width <= 0.0 || height <= 0.0 ||
	    min_mass <= 0.0 || max_mass <= 0.0) {
		fputs("All arguments need to be a positive numbers!\n",
		      stderr);
		goto fail_arg;
	}

	if (min_mass >= max_mass) {
		fputs("Minimum mass needs to less than maximum mass!\n",
		      stderr);
		goto fail_arg;
	}

	if (optind < argc) {
		fputs("Unknown extra arguments.\n", stderr);
		goto fail_arg;
	}

	return;
fail_arg:
	fputs("Try `particle --help' for more information.\n", stderr);
	exit(EXIT_FAILURE);
}

/*****************************************************************************/

static double gen_number(double min, double max)
{
	return min + ((double)rand() / RAND_MAX) * (max - min);
}

static int create_particles(void)
{
	size_t i;

	particles = calloc(sizeof(particles[0]), count);

	if (particles == NULL) {
		perror("allocating particles");
		return -1;
	}

	srand(time(NULL));

	for (i = 0; i < count; ++i) {
		particles[i].x = gen_number(-0.5 * width, 0.5 * width);
		particles[i].y = gen_number(-0.5 * height, 0.5 * height);
		particles[i].mass = gen_number(min_mass, max_mass);
	}

	return 0;
}

static void cleanup_particles(void)
{
	free(particles);
}

static int export_data(size_t step_index)
{
	char buffer[128];
	FILE *fp;

	sprintf(buffer, "step_%06zu.bin", step_index);

	fp = fopen(buffer, "wb");
	if (fp == NULL) {
		perror(buffer);
		return -1;
	}

	/* TODO: error handling */
	fwrite(particles, sizeof(particles[0]), count, fp);
	fflush(fp);
	fclose(fp);
	return 0;
}

static void sim_do_step(void)
{
	double m1, m2, f_mag, r, r2, d_x, d_y, f_x, f_y;
	size_t i, j;

	/* advance position from CURRENT velocity */
	for (i = 0; i < count; ++i) {
		particles[i].x += particles[i].v_x * dt;
		particles[i].y += particles[i].v_y * dt;
	}

	/* compute velocity at the NEXT time step */
	for (i = 0; i < count; ++i) {
		f_x = 0.0;
		f_y = 0.0;

		/* accumulate all the forces */
		for (j = 0; j < count; ++j) {
			if (j == i)
				continue;

			m1 = particles[i].mass;
			m2 = particles[j].mass;

			/* vector TOWARDS the other particle */
			d_x = particles[j].x - particles[i].x;
			d_y = particles[j].y - particles[i].y;

			/* distance and squared distance */
			r2 = d_x * d_x + d_y * d_y;
			r = sqrt(r2);

			/* compute and add the force */
			f_mag = (G * m1 * m2) / r2;

			f_x += (d_x / r) * f_mag;
			f_y += (d_y / r) * f_mag;
		}

		/* add velocity change */
		particles[i].v_x += dt * (f_x / particles[i].mass);
		particles[i].v_y += dt * (f_y / particles[i].mass);
	}
}

int main(int argc, char **argv)
{
	int status = EXIT_FAILURE;
	size_t i;

	process_options(argc, argv);

	if (create_particles())
		return EXIT_FAILURE;

	for (i = 0; i < steps; ++i) {
		if (export_data(i))
			goto out;
		sim_do_step();
	}

	if (export_data(i))
		goto out;

	status = EXIT_SUCCESS;
out:
	cleanup_particles();
	return status;
}
