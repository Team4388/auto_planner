#pragma once

#include "motion.h"
#include "math/bezier.h"
#include "../include/SDL.h"

typedef struct {
	enum {
		PATH_MOTION,
		PATH_BEZIER
	} type;

	union {
		MotionPath motion;
		BezierPath bezier;
	};

	rect bounding_box;
} MultiPath;

#define WRAP_MOTION(path) (MultiPath) { PATH_MOTION, .motion = path };
#define WRAP_BEZIER(path) (MultiPath) { PATH_BEZIER, .bezier = path };

typedef struct {
	Field field;
	float zoom_level;
	int	  x_offset;
	int	  y_offset;

	struct {
		int        cap;
		int        len;
		MultiPath *arr;
		int        selected;
	} paths;

	enum {
		MODE_RECORD,
		MODE_EDIT,
		MODE_PLAYBACK,
	} mode;
} PlannerState;

void init_planner(StartPos start);
void path_playback(bool start, MotionPath *path);
void save_correct_format(MultiPath *path);
void planner_loop(SDL_Event *e);
