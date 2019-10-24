#include "heat_stencil.h"

typedef struct {
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char chksum[8];
	char typeflag;
	char linkname[100];
	char magic[6];
	char version[2];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	char prefix[167];
} tar_header_t;

static void write_binary(char *dst, uint64_t value, int digits)
{
	memset(dst, 0, digits);

	while (digits > 0) {
		((unsigned char *)dst)[digits - 1] = value & 0xFF;
		--digits;
		value >>= 8;
	}

	((unsigned char *)dst)[0] |= 0x80;
}

static void write_number(char *dst, uint64_t value, int digits)
{
	uint64_t mask = 0;
	char buffer[64];
	int i;

	for (i = 0; i < (digits - 1); ++i)
		mask = (mask << 3) | 7;

	if (value <= mask) {
		sprintf(buffer, "%0*o ", digits - 1, (unsigned int)value);
		memcpy(dst, buffer, digits);
	} else if (value <= ((mask << 3) | 7)) {
		sprintf(buffer, "%0*o", digits, (unsigned int)value);
		memcpy(dst, buffer, digits);
	} else {
		write_binary(dst, value, digits);
	}
}

static unsigned int get_tar_checksum(const tar_header_t *hdr)
{
	const unsigned char *header_start = (const unsigned char *)hdr;
	const unsigned char *chksum_start = (const unsigned char *)hdr->chksum;
	const unsigned char *header_end = header_start + sizeof(*hdr);
	const unsigned char *chksum_end = chksum_start + sizeof(hdr->chksum);
	const unsigned char *p;
	unsigned int chksum = 0;

	for (p = header_start; p < chksum_start; p++)
		chksum += *p;
	for (; p < chksum_end; p++)
		chksum += ' ';
	for (; p < header_end; p++)
		chksum += *p;
	return chksum;
}

static int write_tar_header(int fd, size_t frame_number, uint64_t size)
{
	unsigned int chksum;
	tar_header_t hdr;

	memset(&hdr, 0, sizeof(hdr));
	sprintf(hdr.name, "frame_%04zu.ppm", frame_number);
	write_number(hdr.mode, 0644, sizeof(hdr.mode));
	write_number(hdr.uid, 0, sizeof(hdr.uid));
	write_number(hdr.gid, 0, sizeof(hdr.gid));
	write_number(hdr.size, size, sizeof(hdr.size));
	write_number(hdr.mtime, 0, sizeof(hdr.mtime));
	hdr.typeflag = '0';
	memcpy(hdr.magic, "ustar ", sizeof(hdr.magic));
	memcpy(hdr.version, " ", sizeof(hdr.version));
	sprintf(hdr.uname, "%u", 0);
	sprintf(hdr.gname, "%u", 0);
	write_number(hdr.devmajor, 0, sizeof(hdr.devmajor));
	write_number(hdr.devminor, 0, sizeof(hdr.devminor));

	chksum = get_tar_checksum(&hdr);
	sprintf(hdr.chksum, "%06o", chksum);
	hdr.chksum[6] = '\0';
	hdr.chksum[7] = ' ';

	write(fd, &hdr, sizeof(hdr));
	return 0;
}

static const struct {
	uint8_t r, g, b;
} colors[] = {
	{ 0x00, 0x00, 0xFF },
	{ 0x00, 0xFF, 0x00 },
	{ 0xFF, 0xFF, 0x00 },
	{ 0xFF, 0x00, 0x00 },
	{ 0xFF, 0xFF, 0xFF },
};

void gen_image(int fd, size_t frame_number,
	       value_t *data, size_t width, size_t height,
	       value_t min, value_t max)
{
	size_t i, count, x, y;
	value_t f, val, step;
	char header[128];
	uint8_t pixel[3];
	uint64_t size;
	void *temp;

	sprintf(header, "P6\n%zu %zu\n255\n", width, height);
	size = strlen(header) + width * height * 3;
	write_tar_header(fd, frame_number, size);
	write(fd, header, strlen(header));

	count = sizeof(colors) / sizeof(colors[0]);
	step = 1.0 / count;

	for (y = 0; y < height; ++y) {
		for (x = 0; x < width; ++x) {
			val = *(data++);

			if (val < min || val > max) {
				pixel[0] = pixel[1] = pixel[2] = 0x00;
				continue;
			}

			f = (val - min) / (max - min);

			for (i = 0; i < (count - 1); ++i) {
				if (f < (i + 1) * step)
					break;
			}

			f = (f - i * step) / step;

			if (i > 0) {
				pixel[0] = colors[i - 1].r * (1.0 - f);
				pixel[1] = colors[i - 1].g * (1.0 - f);
				pixel[2] = colors[i - 1].b * (1.0 - f);

				pixel[0] += colors[i].r * f;
				pixel[1] += colors[i].g * f;
				pixel[2] += colors[i].b * f;
			} else {
				pixel[0] = colors[i].r;
				pixel[1] = colors[i].g;
				pixel[2] = colors[i].b;
			}

			write(fd, pixel, sizeof(pixel));
		}
	}

	if (size % 512) {
		size = 512 - size % 512;

		temp = calloc(1, size);
		write(fd, temp, size);
		free(temp);
	}
}
