#include "mathutils.h"
#include "../../gfx/gfx.h"
#include <corecrt_math.h>

point scale_to_screen(
		double x,
		double y,
		double w_meters,
		double h_meters,
		double x_off,
		double y_off)
{
	return (point) {
		.x = x * ((double) SCREEN_WIDTH  / w_meters) + x_off,
		.y = y * ((double) SCREEN_HEIGHT / h_meters) + y_off,
	};
}

point scale_to_field(
		double x,
		double y,
		double w_meters,
		double h_meters,
		double x_off,
		double y_off)
{
	return (point) {
		.x = (x - x_off) / ((double) SCREEN_WIDTH  / w_meters),
		.y = (y - y_off) / ((double) SCREEN_HEIGHT / h_meters),
	};
}

bool within_box(point p, rect r) {
	return p.x >= r.x && p.y >= r.y &&
		p.x <= r.x + r.w && p.y <= r.y + r.h;
}

point lerp_point(point a, point b, float p) {
	return (point) {
			a.x + (b.x - a.x) * p,
			a.y + (b.y - a.y) * p
	};
}

double get_distance(point a, point b) {
	return sqrt(
			(a.x - b.x) * (a.x - b.x) +
			(a.y - b.y) * (a.y - b.y));
}
