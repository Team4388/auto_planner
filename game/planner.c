#include "planner.h"
#include "../gfx/gfx.h"
#include "../include/SDL.h"
#include "math/bezier.h"
#include "math/mathutils.h"
#include "motion.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SELECTED state.paths.arr[state.paths.selected]

#define STATE_FIELD_RESCALED state.field.w_meters, state.field.h_meters, 0, 0

static PlannerState state;
static BezierPath bezier = {
	.arr = NULL,
	.cap = 51,
	.len = 0,
	.selected = -1,
};

const point START_POSITIONS[6] = {
	[RED_ONE]    = {14.5, 3.6},
	[RED_TWO]    = {14.5, 5.5},
	[RED_THREE]  = {14.5, 7.0},

	[BLUE_ONE]   = {2.0, 3.6},
	[BLUE_TWO]   = {2.0, 5.5},
	[BLUE_THREE] = {2.0, 7.0},
};

static void build_bounding_box(MultiPath *path) {
	rect box = {
		state.field.startx,
		state.field.starty,
		state.field.startx,
		state.field.starty
	};

	if (path->type == PATH_MOTION) {
		printf("motion.len: %d\n", path->motion.len);
		for (int i = 0; i < path->motion.len; i++) {
			double x = path->motion.odo_path[i].x;
			double y = path->motion.odo_path[i].y;

			if (x < box.x) box.x = x;
			if (y < box.y) box.y = y;
			if (x > box.w) box.w = x;
			if (y > box.h) box.h = y;
		}
	} else {
		for (int i = 0; i < path->bezier.len; i += 2) {
			point prev = path->bezier.arr[i];

			for (float j = 0; j < 1.f; j += RES) {
				point percent_a = lerp_point(path->bezier.arr[i],
						path->bezier.arr[i + 1], j);

				point percent_b = lerp_point(path->bezier.arr[i + 1],
						path->bezier.arr[i + 2], j);

				point curve = lerp_point(percent_a, percent_b, j);
				if (curve.x < box.x) box.x = curve.x;
				if (curve.y < box.y) box.y = curve.y;
				if (curve.x > box.w) box.w = curve.x;
				if (curve.y > box.h) box.h = curve.y;

				prev = curve;
			}
		}
	}

	point ll = scale_to_screen(box.x,         box.y,         STATE_FIELD_RESCALED);
	point rr = scale_to_screen(box.w - box.x, box.h - box.y, STATE_FIELD_RESCALED);
	path->bounding_box = (rect) {ll.x, ll.y, rr.x, rr.y};
}

void init_planner(StartPos start) {
	const size_t path_arr_size = sizeof(MotionPath) * 10;

	state = (PlannerState) {
		.field = {
			.startx    = START_POSITIONS[start].x, // 8.25,
			.starty    = START_POSITIONS[start].y, // 4.05,
			.w_meters  = 16.5,
			.h_meters  = 8.1,
			.xy_speed  = 0.8, //4.8,
			.rot_speed = 0,
		},
		.zoom_level = 1,
		.x_offset   = 0,
		.y_offset   = 0,

		.paths = {
			.cap      = 4,
			.len      = 0,
			.arr      = malloc(path_arr_size),
			.selected = 0,
		},

		.mode = MODE_EDIT,
	};

	memset(state.paths.arr, 0, path_arr_size);

	if (state.paths.arr == NULL) {
		printf("could not allocate MotionPaths");
		exit(1);
	}

	const size_t bezier_size = sizeof(point) * bezier.cap;
	bezier.arr               = malloc(bezier_size);
	memset(bezier.arr, 0, bezier_size);

	bezier.arr[bezier.len++] = (point) {
		state.field.startx,
		state.field.starty
	};

	bezier.arr[bezier.len++] = (point) {
		state.field.startx + 1,
		state.field.starty
	};

	bezier.arr[bezier.len++] = (point) {
		state.field.startx + 2,
		state.field.starty
	};

	state.paths.arr[state.paths.len++] = WRAP_BEZIER(bezier);
	state.paths.arr[state.paths.len++] =
		WRAP_MOTION(load_path("bezier.txt", &state.field));

	for (int i = 0; i < state.paths.len; i++)
		build_bounding_box(&state.paths.arr[i]);
}

static void path_playback(bool start, MotionPath *path) {
	static clock_t start_time = 0;
	static int     index      = 1;

	if (start) {
		start_time = clock();
		index      = 1;
	}

	clock_t playback_time = clock() - start_time;

	while (index < path->len && path->points[index].time < playback_time) {
		index++;
	}

	if (index == path->len) {
		state.mode = MODE_EDIT;
		return;
	}

	double x_min = path->odo_path[index - 1].x;
	double x_max = path->odo_path[index    ].x;

	double y_min = path->odo_path[index - 1].y;
	double y_max = path->odo_path[index    ].y;

	double lerp_time =
		((double) (path->points[index].time - path->points[index-1].time)) /
		((double) playback_time);

	point robot_pos = {
		x_min + lerp_time * (x_max - x_min),
		y_min + lerp_time * (y_max - y_min),
	};

	draw_robot(&state.field, robot_pos);
}

static void bezier_playback(bool start, BezierPath *path) {

}

// TODO: use BezierPath & port to bezier module
static void deselect_bez_point(MultiPath *path) {
	if (path->type == PATH_MOTION) return;
	path->bezier.selected = -1;
	build_bounding_box(path);
}

static void select_path(int x, int y) {
	point mouse = { x, y };

	for (int i = 0; i < state.paths.len; i++) {
		if (within_box(mouse, state.paths.arr[i].bounding_box)) {
			state.paths.selected = i;
			return;
		}
	}
}

static void draw_correct_path(MultiPath *path, bool bold, bool select) {
	if (path->type == PATH_MOTION)
		draw_path(&path->motion, &state.field, bold, select);
	else
		draw_bezier(&path->bezier, &state.field, bold, select);
}

static void draw_paths(int x, int y) {
	point mouse = { x, y };

	for (int i = 0; i < state.paths.len && i < 2; i++) {
		if (state.paths.selected == i) {
			draw_correct_path(&state.paths.arr[i], true, true);
			continue;
		}

		if (within_box(mouse, state.paths.arr[i].bounding_box))
			draw_correct_path(&state.paths.arr[i], true, false);
		else
			draw_correct_path(&state.paths.arr[i], false, false);
	}
}

void save_correct_format(MultiPath *path) {
	if (path->type == PATH_MOTION)
		save_path(&path->motion);
	else
		save_bezier(&path->bezier, "bezier.txt", state.field.xy_speed);
}

void planner_loop(SDL_Event *e) {
	int x, y;
	SDL_GetMouseState(&x, &y);

	while (SDL_PollEvent(e)) {
		switch (e->type) {
			case SDL_QUIT:
				// FIXME: crashing the program to exit is rather ungraceful.
				// I should make a cleanup procedure at some point.
				exit(0);
				break;

			case SDL_MOUSEBUTTONDOWN:
				if (SELECTED.type == PATH_BEZIER &&
						select_bez_point(x, y, &SELECTED.bezier, &state.field)) break;

				select_path(x, y);
				break;

			case SDL_MOUSEBUTTONUP:
				deselect_bez_point(&SELECTED);
				break;

			case SDL_KEYDOWN:
				switch(e->key.keysym.scancode) {
					case SDL_SCANCODE_P:
						if (state.paths.selected >= 0 && SELECTED.type == PATH_MOTION) {
							path_playback(true, &SELECTED.motion);
							state.mode = MODE_PLAYBACK;
						}
						break;

					case SDL_SCANCODE_A:
						if (state.paths.selected >= 0 && SELECTED.type == PATH_BEZIER) {
							if (bezier.len >= bezier.cap) break;

							for (int i = 0; i < 2; i++) {
								SELECTED.bezier.arr[SELECTED.bezier.len++] = (point) {
									.x = state.field.startx,
									.y = state.field.starty,
								};
							}
						}
						break;

					case SDL_SCANCODE_S:
						save_correct_format(&SELECTED);
						break;

					case SDL_SCANCODE_D:
						printf("(%f, %f)\n", bezier.arr[0].x, bezier.arr[0].y);
						break;

					default:
						break;
				}
				break;
		}
	}

	draw_field(&state.field);

	switch (state.mode) {
		case MODE_EDIT:
			if (SELECTED.type == PATH_BEZIER)
				move_point(x, y, &SELECTED.bezier, &state.field);

			draw_paths(x, y);
			draw_robot(&state.field, (point) { state.field.startx, state.field.starty });
			break;
		case MODE_PLAYBACK:
			DRAW_PATH_SELECT(&SELECTED.motion, &state.field);
			path_playback(false, &SELECTED.motion);
			break;
		case MODE_RECORD:
			DRAW_PATH_SELECT(&SELECTED.motion, &state.field);
			draw_robot(&state.field, (point) {
						SELECTED.motion.odo_path[SELECTED.motion.len - 1].x,
						SELECTED.motion.odo_path[SELECTED.motion.len - 1].y,
					});
			break;
	}

	blit_screen();
}
