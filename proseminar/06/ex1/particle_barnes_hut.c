#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <time.h>

#include "particle.h"

/********************************* typedefs **********************************/

typedef struct {
	double x_min;
	double x_max;
	double y_min;
	double y_max;
} area_t;

/****************************** simulation data ******************************/

static double width = 1000.0;
static double height = 1000.0;
static double min_mass = 10.0;
static double max_mass = 100.0;
static double vmin = 0.0;
static double vmax = 10.0;
static double dt = 1.0;
static double threshold = 0.5;
static size_t count = 1000;
static size_t steps = 1000;
static double G = 1.0;//6.67430e-11;
static area_t global_area = {-500, 500, -500, 500};
static particle_t *particles = NULL;
static particle_t *tree = NULL;

/************************** command line processing **************************/

static struct option long_opts[] = {
	{ "field-width", required_argument, NULL, 'x' },
	{ "field-height", required_argument, NULL, 'y' },
	{ "count", required_argument, NULL, 'c' },
	{ "min-mass", required_argument, NULL, 'm' },
	{ "max-mass", required_argument, NULL, 'M' },
	{ "steps", required_argument, NULL, 's' },
	{ "time", required_argument, NULL, 't' },
	{ "min-speed", required_argument, NULL, 'u' },
	{ "max-speed", required_argument, NULL, 'U' },
	{ "threshold", required_argument, NULL, 'T' },
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
};

static const char *short_opts = "x:y:c:m:M:s:t:u:U:T:hV";

static const char *usagestr =
"Usage: particle_barnes_hut [OPTIONS...]\n"
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
"  --max-mass, -M <number>      Upper threshold for randomized\n"
"                               particle mass.\n"
"  --steps, -s <number>         How many simulation steps to compute.\n"
"  --time, -t <number>          Time difference for each step.\n"
"  --min-speed, -u <number>     Lower threshold for randomized\n"
"                               particle velocity.\n"
"  --max-speed, -U <number>     Upper threshold for randomized\n"
"                               particle velocity.\n"
"  --threshold, -T <number>     Threshold for clustering of particles,\n"
"                               default is 0.5, must be in range 0.0 .. 1.0.\n"
"\n"
"  --help, -h                   Display this help text and exit.\n"
"  --version, -V                Display version information and exit.\n"
"\n";

#define GPL_URL "https://gnu.org/licenses/gpl.html"

static const char *versionstr =
"particle (the serial one, method after Barnes and Hut) 0.2\n"
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
			global_area.x_max = width/2;
			global_area.x_min = -global_area.x_max;
			break;
		case 'y':
			height = strtod(optarg, &end);
			global_area.y_max = height/2;
			global_area.y_min = -global_area.y_max;
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
		case 'u':
			vmin = strtod(optarg, &end);
			break;
		case 'U':
			vmax = strtod(optarg, &end);
			break;
		case 'T':
			threshold = strtod(optarg, &end);
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
		fputs("Minimum mass needs to be less than maximum mass!\n",
		      stderr);
		goto fail_arg;
	}

	if (vmin > vmax) {
		fputs("Minimum speed needs to be less or equal than maximum speed!\n",
		      stderr);
		goto fail_arg;
	}

	if (threshold < 0.0 || threshold > 1.0) {
		fputs("Threshold must be in range 0.0 .. 1.0!\n",
		      stderr);
		goto fail_arg;
	}

	if (optind < argc) {
		fputs("Unknown extra arguments.\n", stderr);
		goto fail_arg;
	}

	return;
fail_arg:
	fputs("Try `particle_barnes_hut --help' for more information.\n", stderr);
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
		particles[i].v_x = gen_number(vmin, vmax); // non uniform limit: vmax in diagonal direction is approx. 1.4*vmax
		particles[i].v_y = gen_number(vmin, vmax);
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

static particle_t** quadrant(particle_t *tree, const particle_t *newLeaf, area_t *area)
{
	double x_border = area->x_min + (area->x_max - area->x_min) / 2;
	double y_border = area->y_min + (area->y_max - area->y_min) / 2;
	if (newLeaf->x > x_border) {
		if (newLeaf->y > area->y_min + (area->y_max - area->y_min) / 2) {
			area->x_min = x_border;
			area->y_min = y_border;
			return &(tree->south);
		} else {
			area->x_min = x_border;
			area->y_max = y_border;
			return &(tree->west);
		}
	} else {
		if (newLeaf->y > y_border) {
			area->x_max = x_border;
			area->y_min = y_border;
			return &(tree->east);
		} else {
			area->x_max = x_border;
			area->y_max = y_border;
			return &(tree->north);
		}
	}
}

void updateValues(particle_t *current, const particle_t *newLeaf)
{
	current->x = current->mass * current->x + newLeaf->mass * newLeaf->x;
	current->y = current->mass * current->y + newLeaf->mass * newLeaf->y;
	current->mass += newLeaf->mass;
	current->x /= current->mass;
	current->y /= current->mass;
}

int is_leaf(particle_t *tree)
{
	return (tree->north == NULL && tree->west == NULL && tree->east == NULL && tree->south == NULL);
}

particle_t *new_node() {
	return calloc(1, sizeof(particle_t));
}

void cleanup_nodes() {
	// FIXME implement
}

static int insertNode(particle_t *tree, particle_t *newLeaf, area_t *area) // FIXME invalid for root node = NULL
{
	particle_t* current;
	particle_t** p_current = &tree;
	do {
		current = *p_current;
		updateValues(current, newLeaf);
		p_current = quadrant(current, newLeaf, area);
	} while (*p_current != NULL && !is_leaf(*p_current));
	if(*p_current == NULL) {
		newLeaf->parent = current;
		*p_current = newLeaf;
	} else {
		particle_t* temp = *p_current;
		*p_current = new_node();
		(*p_current)->parent = current;
		current = *p_current;
		temp->parent = current;
		updateValues(current, temp);
		p_current = quadrant(current, temp, area);
		*p_current = temp;
		insertNode(current, newLeaf, area);
	}
	return 0; // success // TODO error handling
}


static particle_t** cond_next(particle_t **node, particle_t *current, int *level, int *last_to_parent) {
	double s, d;
	s = fmax(width, height) / *level;
	d = fabs(fmax(current->x - (*node)->x, current->y - (*node)->y)); // infinity norm
	if((is_leaf(*node) || last_to_parent) && node == &((*node)->parent->west)) {
		--(*level);
		*last_to_parent = 1;
		return &((*node)->parent);
	} else if (!is_leaf(*node) && !last_to_parent && (d == 0 || s/d >= threshold)) {
		++(*level);
		*last_to_parent = 0;
		return &((*node)->north);
	} else {
		*last_to_parent = 0;
		return node + sizeof(particle_t*);
	}
}

static particle_t** traverse(particle_t **node, particle_t *current, int *level, int *last_to_parent) {
	double s, d;
	s = fmax(width, height) / *level;
	d = fabs(fmax(current->x - (*node)->x, current->y - (*node)->y)); // infinity norm
	particle_t **ret = NULL;
	while ((*node == NULL || (*node)->parent != NULL) && (d != 0 && s/d < threshold)) {
		ret = cond_next(node, current, level, last_to_parent);
	}
	return ret;
}

static void sim_do_step(void)//FIXME
{
	double m1, m2, f_mag, r, r2, d_x, d_y, f_x, f_y;
	size_t i;
	particle_t ** node;
	int level, last2parent;

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
		node = &tree;
		level = 1;
		last2parent = 0;
		while((node = traverse(node, &(particles[i]), &level, &last2parent)) != NULL) {

			m1 = particles[i].mass;
			m2 = (*node)->mass;

			/* vector TOWARDS the other particle */
			d_x = (*node)->x - particles[i].x;
			d_y = (*node)->y - particles[i].y;

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
	area_t area;

	process_options(argc, argv);

	if (create_particles())
		return EXIT_FAILURE;

	tree = new_node();
	for (i = 0; i < count; ++ i) {
		area.x_min = global_area.x_min;
		area.x_max = global_area.x_max;
		area.y_min = global_area.y_min;
		area.y_max = global_area.y_max;
		insertNode(tree, &particles[i], &area);
	}

	for (i = 0; i < steps; ++i) {
		if (export_data(i))
			goto out;
		sim_do_step();
	}

	if (export_data(i))
		goto out;

	status = EXIT_SUCCESS;
out:
	cleanup_nodes();
	cleanup_particles();
	return status;
}
