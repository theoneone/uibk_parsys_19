#ifndef HEAT_STENCIL_H
#define HEAT_STENCIL_H

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>


typedef double value_t;

#define RESOLUTION 120


void gen_image(int fd, size_t frame_number,
	       value_t *data, size_t width, size_t height,
	       value_t min, value_t max);

#endif /* HEAT_STENCIL_H */
