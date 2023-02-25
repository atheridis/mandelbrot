/*
 * Creates an image of the mandelbrot set.
 * Copyright (C) 2023  Georgios Atheridis <georgios@atheridis.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <complex.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <threads.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define COLOR_DEPTH 3
#define MANDELBROT_ESCAPE 4


struct rgb {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

struct rgb hue2rgb(const int_fast16_t hue)
{
	struct rgb rgb_color;

	switch ((hue % 360) / 60) {
	case 0:
		rgb_color.r = UINT8_MAX;
		rgb_color.g = 4 * hue + hue / 4;
		rgb_color.b = 0;
		break;
	case 1:
		rgb_color.r = 4 * (120 - hue) + (120 - hue) / 4;
		rgb_color.g = UINT8_MAX;
		rgb_color.b = 0;
		break;
	case 2:
		rgb_color.r = 0;
		rgb_color.g = UINT8_MAX;
		rgb_color.b = 4 * (hue - 120) + (hue - 120) / 4;
		break;
	case 3:
		rgb_color.r = 0;
		rgb_color.g = 4 * (240 - hue) + (240 - hue) / 4;
		rgb_color.b = UINT8_MAX;
		break;
	case 4:
		rgb_color.r = 4 * (hue - 240) + (hue - 240) / 4;
		rgb_color.g = 0;
		rgb_color.b = UINT8_MAX;
		break;
	case 5:
		rgb_color.r = UINT8_MAX;
		rgb_color.g = 0;
		rgb_color.b = 4 * (360 - hue) + (360 - hue) / 4;
		break;
	}

	return rgb_color;
}


struct fractal_definition {
	const int_fast32_t rows;
	const int_fast32_t cols;
	const double complex center;
	double width;
	int_fast32_t steps;
};

int_fast32_t mandelbrot(const double complex c, const int_fast32_t steps)
{
	double complex z = c;
	for (int_fast32_t i = 0; i < steps; i++) {
		if (creal(z) * creal(z) + cimag(z) * cimag(z) > MANDELBROT_ESCAPE) {
			return i;
		}
		z = z * z + c;
	}
	return 0;
}

void compute_colors(const struct fractal_definition *const fractal, uint8_t *result)
{
	const double aspect_ratio = fractal->width / fractal->cols;
	const double center_real = creal(fractal->center);
	double complex c;
	double c_imag;

	int_fast32_t steps_till_escaped;
	struct rgb rgb_color;

	for (double y = 0; y < fractal->rows; y++) {
		printf("y = %g out of %" PRIuFAST32 "\n", y + 1, (*fractal).rows);
		c_imag = aspect_ratio * (y - (fractal->rows - 1) / 2.0);
		for (double x = 0; x < fractal->cols; x++) {
			c = aspect_ratio * x - fractal->width * 0.5 + center_real + c_imag * I;
			if ((steps_till_escaped = mandelbrot(c, fractal->steps))) {
				rgb_color = hue2rgb(steps_till_escaped * 3.6);
			} else {
				rgb_color.r = rgb_color.g = rgb_color.b = 0;
			}
			*result++ = rgb_color.r;
			*result++ = rgb_color.g;
			*result++ = rgb_color.b;
		}
	}
}

int main(int argc, char **argv)
{
	char *file_name = "mandelbrot.png";
	if (argc > 1) {
		file_name = strcat(argv[1], ".png");
	}

	struct fractal_definition fractal = {
		.rows = 1080,
		.cols = 1920,
		.center = 0.0 + 0.0 * I,
		.width = 4.0,
		.steps = 100,
	};

	uint8_t *fractal_result = malloc(fractal.cols * fractal.rows * COLOR_DEPTH);
	compute_colors(&fractal, fractal_result);
	stbi_write_png(file_name, fractal.cols, fractal.rows, 3, fractal_result, fractal.cols * 3);
	free(fractal_result);

	return EXIT_SUCCESS;
}
