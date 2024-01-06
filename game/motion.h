#pragma once

#include "math/mathutils.h"
#include "../include/SDL.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum {
	RED_ONE,
	RED_TWO,
	RED_THREE,

	BLUE_ONE,
	BLUE_TWO,
	BLUE_THREE,
} StartPos;

typedef struct {
	double startx, starty;
	double w_meters;
	double h_meters;

	double xy_speed; // meters per second
	double rot_speed; // radians per second
} Field;

typedef struct {
	double   leftx;
	double   lefty;
	double   rightx;
	double   righty;
	uint32_t time;
} MotionPoint;

#define PATH_INIT_SIZE 700
typedef struct {
	char        *name;

	MotionPoint	*points;
	uint32_t     cap;
	uint32_t     len;
	point		*odo_path;
	SDL_Rect     bounding_box;
} MotionPath;

MotionPath load_path(const char *filename, Field *field);
void       save_path(MotionPath *path);
bool       free_path(MotionPath *path);

void build_odo_mpath(MotionPath *path, Field *field);
// void build_odo_bpath(MotionPath *path, Bezier *beziers);
