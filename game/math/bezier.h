#pragma once

#include "mathutils.h"
#include "../motion.h"

typedef point Bezier[3];

typedef struct {
	point *arr;
	int    cap;
	int    len;
	int    selected;
} BezierPath;

#define BEZIER_TO_PATH(path) make_bezier_path(3, path)
#define RES 0.025

BezierPath make_bezier_path(int n, point bezier[n]);
void       free_bezier(BezierPath *path);

// is this actually needed?
// maybe, depending on how I do playback and stuff
double get_bez_distance(BezierPath *bezier, double res);
bool   select_bez_point(int x, int y, BezierPath *path, Field *field);
void   move_point(int x, int y, BezierPath *path, Field *field);

// TODO: void deselect_bez_point(BezierPath *path);

void save_bezier(BezierPath *path, const char *filename, double speed);
