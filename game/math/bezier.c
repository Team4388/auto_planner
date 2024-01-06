#include "bezier.h"
#include "mathutils.h"
#include "../motion.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BezierPath make_bezier_path(int n, point bezier[n]) {
	BezierPath path = {
		.arr = malloc(sizeof(point) * n),
		.cap = n,
		.len = n,
	};

	memcpy(path.arr, bezier, sizeof(point) * n);

	return path;
}

void free_bezier(BezierPath *path) {
	free(path->arr);
	*path = (BezierPath) {
		.arr      = NULL,
		.cap      = 0,
		.len      = 0,
		.selected = -1,
	};
}

double get_bez_distance(BezierPath *bezier, double res) {
	double dist = 0;

	for (int i = 0; i < bezier->len; i += 2) {
		point prev = bezier->arr[i];

		for (float j = 0; j < 1.f; j += res) {
			point percent_a = lerp_point(bezier->arr[i    ], bezier->arr[i + 1], j);
			point percent_b = lerp_point(bezier->arr[i + 1], bezier->arr[i + 2], j);

			point curve = lerp_point(percent_a, percent_b, j);
			dist += get_distance(prev, curve);

			prev = curve;
		}
	}

	return dist;
}

bool select_bez_point(int x, int y, BezierPath *path, Field *field) {
	for (int i = 0; i < path->len; i++) {
		point node = scale_to_screen(
				path->arr[i].x,
				path->arr[i].y,
				FIELD_RESCALED);
		rect handle = { node.x - 5, node.y - 5, 10, 10 };

		if (within_box((point) { x, y }, handle)) {
			path->selected = i;
			return true;
		}
	}

	return false;
}

void move_point(int x, int y, BezierPath *path, Field *field) {
	if (path->selected == -1) return;
	path->arr[path->selected] = scale_to_field(x, y, FIELD_RESCALED);
}

#define DIST    0.01
#define DIST_SQ (DIST * DIST)

/* fuck how do I do this shit?
 * the issue is traveling a consistent distance along the curve
 * instead of a percentage of each bezier.
 *
 * this is a very slow implementation of `arc length parameterization`
 *
 * I don't factor in ramping, which could lead to some problems */
void save_bezier(BezierPath *path, const char *filename, double speed) {
	if (speed < DIST) {
		printf("bezier cannot be exported with speed %f\n", speed);
		return;
	}

	FILE *file;
	fopen_s(&file, filename, "w+");
	fprintf(file, "%f,%f,%f,%f,%d\n", 0.f, 0.f, 0.f, 0.f, 0);

	uint32_t time_ms = 0;

	for (int i = 0; i < path->len-1; i += 2) {
		point prev = path->arr[i];

		for (float j = 0; j < 1.f; j += 0.001) {
			point percent_a = lerp_point(path->arr[i    ], path->arr[i + 1], j);
			point percent_b = lerp_point(path->arr[i + 1], path->arr[i + 2], j);

			point curve = lerp_point(percent_a, percent_b, j);

			double dist_sq =
				(prev.x - curve.x) * (prev.x - curve.x) +
				(prev.y - curve.y) * (prev.y - curve.y);

			if (dist_sq > DIST_SQ) {
				// are the component distances correct or should I apply normalization?
				double dist   = sqrt(dist_sq);
				double dist_x = curve.x - prev.x;
				double dist_y = curve.y - prev.y;

				// does this make sense? am i going insane?
				double time_sec = dist / speed;
				double left_x   = (dist_y / time_sec) / speed;
				double left_y   = (dist_x / time_sec) / speed;

				time_ms += (uint32_t) (time_sec * 1000);

				fprintf(file, "%f,%f,%f,%f,%d\n",
						left_x, left_y, 0.f, 0.f, time_ms);
				prev = curve;
			}
		}
	}

	fclose(file);
	printf("saved bezier path %s\n", filename);
}
