#pragma once

#include <stdbool.h>

typedef struct {
	double x, y;
} point;

typedef struct {
	double x, y, w, h;
} rect;

#define FIELD_RESCALED field->w_meters, field->h_meters, 0, 0

// Rescales field coordinates to pixels
point scale_to_screen(
		double x,
		double y,
		double w_meters,
		double h_meters,
		double x_off,
		double y_off);

// Rescales pixel coordinates to meters
point scale_to_field(
		double x,
		double y,
		double w_meters,
		double h_meters,
		double x_off,
		double y_off);

bool within_box(point p, rect r);
point lerp_point(point a, point b, float p);
double get_distance(point a, point b);
