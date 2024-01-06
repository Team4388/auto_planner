#include "motion.h"
#include "math/mathutils.h"
#include "planner.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool add_point(MotionPath *path, double input[4], uint32_t time) {
	if (path->len >= path->cap) {
		path->cap *= 2;
		printf("new cap %d\n", path->cap);

		MotionPoint *tmp   = path->points;
		const size_t psize = sizeof(MotionPoint) * path->cap;
		path->points       = malloc(psize);
		memset(path->points, 0, psize);

		memcpy(path->points, tmp, sizeof(MotionPoint) * (path->len));
		free(tmp);

		if (path->points == NULL) {
			printf("reallocation failed in add_point\n");
			free(path->points);
			return false;
		}
	}

	path->points[path->len++] = (MotionPoint) {
		.leftx	= input[0],
		.lefty	= input[1],
		.rightx	= input[2],
		.righty	= input[3],
		.time	= time,
	};

	return true;
}

MotionPath load_path(const char *filename, Field *field) {
	FILE *file;
	fopen_s(&file, filename, "r");

	const int    filename_len = strlen(filename) + 1;
	const size_t psize        = sizeof(MotionPoint) * PATH_INIT_SIZE;

	MotionPath path = {
		.name		  = malloc(sizeof(char) * filename_len),
		.points		  = malloc(psize),
		.cap		  = PATH_INIT_SIZE,
		.len		  = 0,
		.odo_path	  = NULL,
		.bounding_box = { 0, 0, 0, 0 },
	};

	strcpy_s(path.name, filename_len, filename);
	memset(path.points, 0, psize);

	char num_buf[200];
	int  num_top = 0;

	double point_data[4];
	int    point_top = 0;

	char ch;
	while ((ch = fgetc(file)) != EOF) {
		switch (ch) {
			case '-':
			case '.':
			case '0' ... '9':
				num_buf[num_top++] = ch;
				break;
			case ',':
				num_buf[num_top++] = '\0';
				point_data[point_top++] = atof(num_buf);
				num_top = 0;
				break;
			case '\n':
				num_buf[num_top++] = '\0';
				add_point(&path, point_data, atoi(num_buf));

				num_top   = 0;
				point_top = 0;
				break;
			default:
				break;
		}
	}

	fclose(file);

	build_odo_mpath(&path, field);
	return path;
}

// PERF: you know your data is well structured
// when serialization is this easy
void save_path(MotionPath *path) {
	FILE *file;
	fopen_s(&file, path->name, "w+");

	for (int i = 0; i < path->len; i++) {
		fprintf(file, "%f,%f,%f,%f,%d\n",
				path->points[i].leftx,
				path->points[i].lefty,
				path->points[i].rightx,
				path->points[i].righty,
				path->points[i].time);
	}

	fclose(file);
	printf("saved motion path %s\n", path->name);
}

bool free_path(MotionPath *path) {
	if (path == NULL) return false;
	free(path->name);
	free(path->points);
	path->name   = NULL;
	path->points = NULL;
	return true;
}

static inline double motion_curve(double i) {
	return i;
}

void build_odo_mpath(MotionPath *path, Field *field) {
	if (path->odo_path != NULL) return;

	const size_t path_size = sizeof(point) * path->len;
	path->odo_path         = malloc(path_size);
	memset(path->odo_path, 0, path_size);

	printf("building odo mpath\n");

	double x = field->startx;
	double y = field->starty;

	path->odo_path[0] = (point) {
		.x = field->startx,
		.y = field->starty,
	};

	for (int i = 1; i < path->len; i++) {
		// get the inputs (should represent the derivative of our auto)

		double ix = path->points[i].leftx;
		double iy = path->points[i].lefty;

		// multiply by speed

		double dx = iy * field->xy_speed;
		double dy = ix * field->xy_speed;

		// take the time difference (in seconds) and multiply the derivative by it

		dx *= (path->points[i].time - path->points[i-1].time) / 1000.f;
		dy *= (path->points[i].time - path->points[i-1].time) / 1000.f;

		/* overall, for the x axis the full equation should look like this:
		 * (leftx_input * speed) * ((time_ms1 - time_ms0) / 1000)
		 *
		 * a few other useful facts:
		 *     input * speed = distance
		 * */

		path->odo_path[i] = (point) {x, y};

		// change x & y by the derivative

		x += dx;
		y += dy;
	}

	printf("built odo mpath\n");
}
