#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdint.h>
#include <endian.h>
#include <stdio.h>
#include <errno.h>
#include <zlib.h>

#include "particle.h"

/*************************** simulation parameters ***************************/

static size_t img_width = 1000;
static size_t img_height = 1000;

static double width = 1000.0;
static double height = 1000.0;
static double min_mass = 10.0;
static double max_mass = 100.0;

/************************** command line processing **************************/

static struct option long_opts[] = {
	{ "field-width", required_argument, NULL, 'x' },
	{ "field-height", required_argument, NULL, 'y' },
	{ "image-width", required_argument, NULL, 'X' },
	{ "image-height", required_argument, NULL, 'Y' },
	{ "min-mass", required_argument, NULL, 'm' },
	{ "max-mass", required_argument, NULL, 'M' },
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
};

static const char *short_opts = "x:y:X:Y:m:M:hV";

static const char *usagestr =
"Usage: bin2png [OPTIONS...]\n"
"\n"
"Convert a data dump from `particle` to a PNG image.\n"
"\n"
"The given field dimensions are linearly mapped to the image dimensions.\n"
"Particle mass thresholds are used for colorization.\n"
"\n"
"The input data is read from STDIN, the output PNG image is written to\n"
"STDOUT.\n"
"\n"
"Possible options:\n"
"\n"
"  --field-width, -x <number>   The width of the particle field.\n"
"  --field-height, -y <number>  The height of the particle field.\n"
"  --image-width, -X <number>   The width of the resulting image.\n"
"  --image-height, -Y <number>  The height of the resulting image.\n"
"  --min-mass, -m <number>      Lower threshold for particle mass.\n"
"  --max-mass, -M <number>      Upper threshold for particle mass.\n"
"\n"
"  --help, -h                   Display this help text and exit.\n"
"  --version, -V                Display version information and exit.\n"
"\n"
"Example:\n"
"\n"
"    $ particle\n"
"    $ bin2png < step_42.bin > step_42.png\n"
"\n";

#define GPL_URL "https://gnu.org/licenses/gpl.html"

static const char *versionstr =
"bin2png (particle) 0.1\n"
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
		case 'X':
			img_width = strtol(optarg, &end, 10);
			break;
		case 'Y':
			img_height = strtol(optarg, &end, 10);
			break;
		case 'm':
			min_mass = strtod(optarg, &end);
			break;
		case 'M':
			max_mass = strtod(optarg, &end);
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

	if (img_width < 1 || img_height < 1 || width <= 0.0 || height <= 0.0 ||
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

/******************************* PNG exporter *******************************/

enum {
	PNG_RGB = 2,
	PNG_RGBA = 6,
};

typedef struct {
	int format;
	uint32_t width;
	uint32_t height;
	uint8_t *data;
} png_t;

typedef struct {
	uint32_t size;
	char id[4];
} chunk_hdr_t;

typedef struct {
	uint32_t width;
	uint32_t height;
	uint8_t bit_per_color;
	uint8_t type;
	uint8_t compression;
	uint8_t filtering;
	uint8_t flags;
} png_hdr_t;

static void write_chunk(FILE *fp, const char id[4],
			const uint8_t *data, uint32_t size)
{
	chunk_hdr_t header;
	uint32_t crc;

	header.size = htobe32(size);
	memcpy(header.id, id, 4);
	fwrite(&header, sizeof(header), 1, fp);

	crc = crc32(0, (const void *)id, 4);

	if (data != NULL) {
		fwrite(data, 1, size, fp);
		crc = crc32(crc, data, size);
	}

	crc = htobe32(crc);
	fwrite(&crc, sizeof(crc), 1, fp);
}

static void write_header(const png_t *png, FILE *fp)
{
	png_hdr_t hdr;

	memset(&hdr, 0, sizeof(hdr));
	hdr.width = htobe32(png->width);
	hdr.height = htobe32(png->height);
	hdr.bit_per_color = 8;
	hdr.type = png->format;

	write_chunk(fp, "IHDR", (const uint8_t *)&hdr, 13);
}

static int write_image(const png_t *png, FILE *fp)
{
	unsigned int bpp = png->format == PNG_RGBA ? 4 : 3;
	uint8_t *buffer;
	z_stream strm;
	int ret = -1;
	size_t size;

	memset(&strm, 0, sizeof(strm));
	if (deflateInit(&strm, 9) != Z_OK) {
		fputs("internal error creating zlib stream\n", stderr);
		return -1;
	}

	size = png->width * png->height * bpp + png->height;
	buffer = malloc(size * 2);

	if (buffer == NULL) {
		perror("allocating scratch buffer for PNG compression");
		goto out_buf;
	}

	strm.next_in = (void *)png->data;
	strm.avail_in = size;
	strm.next_out = buffer;
	strm.avail_out = size * 2;

	if (deflate(&strm, Z_FINISH) != Z_STREAM_END) {
		fputs("gzip block processing failed\n", stderr);
		goto out;
	}

	write_chunk(fp, "IDAT", buffer, strm.total_out);
	ret = 0;
out:
	free(buffer);
out_buf:
	deflateEnd(&strm);
	return ret;
}

static void png_to_file(const png_t *png, FILE *fp)
{
	fwrite("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 1, 8, fp);
	write_header(png, fp);
	write_image(png, fp);
	write_chunk(fp, "IEND", NULL, 0);
}

static int png_init(png_t *png, int format, uint32_t width, uint32_t height)
{
	size_t bpp = format == PNG_RGB ? 3 : 4;

	memset(png, 0, sizeof(*png));
	png->format = format;
	png->width = width;
	png->height = height;
	png->data = calloc(1, width * height * bpp + height);

	if (png->data == NULL) {
		perror("png init");
		return -1;
	}

	return 0;
}

static void png_cleanup(png_t *png)
{
	free(png->data);
	memset(png, 0, sizeof(*png));
}

/*****************************************************************************/

int main(int argc, char **argv)
{
	int status = EXIT_FAILURE;
	uint8_t r, g, b;
	particle_t p;
	size_t x, y;
	png_t png;
	double f;

	process_options(argc, argv);

	if (png_init(&png, PNG_RGB, img_width, img_height))
		return EXIT_FAILURE;

	for (;;) {
		if (fread(&p, sizeof(p), 1, stdin) != 1) {
			if (feof(stdin))
				break;

			if (ferror(stdin)) {
				fputs("Error reading from STDIN.\n", stderr);
				goto out;
			}

			fputs("Truncated read on STDIN.\n", stderr);
			goto out;
		}

		if (p.x < (-0.5 * width) || p.y < (-0.5 * height))
			continue;

		if (p.x > (0.5 * width) || p.y > (0.5 * height))
			continue;

		x = img_width * ((p.x + width * 0.5) / width);
		y = img_height * ((p.y + height * 0.5) / height);

		if (x >= img_width || y >= img_height)
			continue;

		f = (p.mass - min_mass) / (max_mass - min_mass);

		r = f * 255.0;
		g = (1.0 - f) * 255.0;
		b = 0.0;

		png.data[y * (img_width * 3 + 1) + x * 3    ] = b;
		png.data[y * (img_width * 3 + 1) + x * 3 + 1] = g;
		png.data[y * (img_width * 3 + 1) + x * 3 + 2] = r;
	}

	png_to_file(&png, stdout);

	status = EXIT_SUCCESS;
out:
	png_cleanup(&png);
	return status;
}
