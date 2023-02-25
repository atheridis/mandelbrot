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
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define COLOR_DEPTH 3
#define MANDELBROT_ESCAPE 4
#define THREAD_COUNT 30


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
	const double width;
	const int_fast32_t steps;
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

struct frame {
	int_fast32_t frame_number;
	double zoom_speed;
	const struct fractal_definition *fractal;
};

void compute_colors(const struct frame *const f, uint8_t *result)
{
	const double new_width = f->fractal->width * pow(f->zoom_speed, f->frame_number);
	const double resolution = new_width / f->fractal->cols;
	const double center_real = creal(f->fractal->center);
	const double center_imag = cimag(f->fractal->center);
	double complex c;
	double c_imag;

	int_fast32_t steps_till_escaped;
	struct rgb rgb_color;

	for (double y = 0; y < f->fractal->rows; y++) {
		c_imag = resolution * (y - (f->fractal->rows - 1) / 2.0) + center_imag;
		for (double x = 0; x < f->fractal->cols; x++) {
			c = resolution * x - new_width * 0.5 + center_real + c_imag * I;
			if ((steps_till_escaped =
			             mandelbrot(c, f->fractal->steps + 7 * f->frame_number))) {
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

void *create_frame(void *args)
{
	const struct frame *f = args;
	char file_name[32];
	sprintf(file_name, "frames/frame_%05" PRIuFAST32 ".png", f->frame_number);

	uint8_t *fractal_result = malloc(f->fractal->cols * f->fractal->rows * COLOR_DEPTH);
	compute_colors(f, fractal_result);
	stbi_write_png(file_name, f->fractal->cols, f->fractal->rows, 3, fractal_result,
	               f->fractal->cols * 3);
	free(fractal_result);
	return NULL;
}

int main(void)
{
	const struct fractal_definition fractal = {
		.rows = 1080,
		.cols = 1920,
		.center = -0.55166952885424866 + 0.62569262930977338 * I,
		.width = 2.5,
		.steps = 200,
	};
	const int_fast32_t max_frames = 500;
	pthread_t t[THREAD_COUNT];
	struct frame *frames[THREAD_COUNT];
	mkdir("frames", 0775);

	for (int_fast32_t i = 0; i < max_frames / THREAD_COUNT + 1; i++) {
		for (int_fast32_t thread = 0; thread < THREAD_COUNT; thread++) {
			printf("frame: %ld out of %ld\n", i * THREAD_COUNT + thread + 1, THREAD_COUNT * (max_frames / THREAD_COUNT + 1));
			frames[thread] = malloc(sizeof *frames[thread]);
			frames[thread]->frame_number = i * THREAD_COUNT + thread;
			frames[thread]->zoom_speed = 0.98;
			frames[thread]->fractal = &fractal;
			pthread_create(t + thread, NULL, create_frame, frames[thread]);
		}

		for (int_fast32_t thread = 0; thread < THREAD_COUNT; thread++) {
			pthread_join(t[thread], NULL);
			free(frames[thread]);
		}
	}

	return EXIT_SUCCESS;
}
