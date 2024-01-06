#include "gfx.h"
#include "../game/motion.h"
#include "../include/SDL.h"
#include "../include/SDL_render.h"
#include "../include/SDL_image.h"
#include <corecrt.h>
#include <stdbool.h>
#include <stdio.h>

static Window window;

static void draw_line_width(SDL_Renderer *r, point a, point b, float width) {
	// HACK: really bad way of drawing lines: draws width ^ 2 lines instead.
	// I should find a way to do it with OpenGL instead with basic mesh rendering.
	for (float i = 0; i < width; i += 1.f) {
		for (float j = 0; j < width; j += 1.f) {
			SDL_RenderDrawLine(r, (a.x - (width/2) + i), (a.y - (width/2) + j),
					(b.x - (width/2) + i), (b.y - (width/2) + j));
		}
	}
}

errno_t init_gfx(bool funny) {
	if (SDL_Init(SDL_FLAGS) != 0) {
		printf("could not initialize sdl2\n");
		exit(1);
	}

	if (IMG_Init(IMG_FLAGS) != IMG_FLAGS) {
		printf("could not initialize images\n");
		exit(1);
	}

	window.window = SDL_CreateWindow("Auto Planner | 4388",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	window.surface = SDL_GetWindowSurface(window.window);
	window.renderer = SDL_CreateRenderer(window.window, -1, RENDER_FLAGS);

	if (window.renderer == NULL) {
		printf("could not create renderer\n");
		exit(1);
	}

	window.texture = IMG_LoadTexture(window.renderer,
			funny ? "deeeen2.jpg" : "field.png");
	if (window.texture == NULL) {
		printf("cannot find texture\n");
		exit(1);
	}

	return 0;
}

void draw_field(Field *field) {
	SDL_SetRenderDrawColor(window.renderer, 0x18, 0x00, 0x00, 0xFF);
	SDL_RenderClear(window.renderer);

	int err = SDL_RenderCopy(window.renderer, window.texture,
			NULL, &(SDL_Rect) { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT });

	if (err < 0) {
		printf("cannot copy texture\n");
		exit(1);
	}
}

void draw_path(MotionPath *path, Field *field, bool bold, bool select) {
	if (path->odo_path == NULL) return;

	if (select)
		SDL_SetRenderDrawColor(window.renderer, 0x00, 0x00, 0xFF, 0xFF);
	else
		SDL_SetRenderDrawColor(window.renderer, 0xFF, 0x00, 0x00, 0xFF);

	double x = path->odo_path[0].x;
	double y = path->odo_path[0].y;

	for (int i = 1; i < path->len; i++) {
		double _x = path->odo_path[i].x;
		double _y = path->odo_path[i].y;

		point a = scale_to_screen( x,  y, FIELD_RESCALED);
		point b = scale_to_screen(_x, _y, FIELD_RESCALED);
		draw_line_width(window.renderer, a, b, bold ? 6.f : 3.f);
		x = _x, y = _y;
	}
}

void draw_bezier(BezierPath *path, Field *field, bool bold, bool select) {
	if (path->arr == NULL && path->len > 0) return;

	if (select)
		SDL_SetRenderDrawColor(window.renderer, 0x00, 0x00, 0xFF, 0xFF);
	else
		SDL_SetRenderDrawColor(window.renderer, 0xFF, 0x00, 0x00, 0xFF);

	for (int i = 0; i < path->len-1; i += 2) {
		point prev = path->arr[i];

		for (float j = 0; j < 1.f; j += RES) {
			point percent_a = lerp_point(path->arr[i    ], path->arr[i + 1], j);
			point percent_b = lerp_point(path->arr[i + 1], path->arr[i + 2], j);

			point curve        = lerp_point(percent_a, percent_b, j);
			point scaled_prev  = scale_to_screen(prev.x, prev.y, FIELD_RESCALED);
			point scaled_curve = scale_to_screen(curve.x, curve.y, FIELD_RESCALED);

			draw_line_width(window.renderer, scaled_prev, scaled_curve,
					bold ? 6.f : 3.f);

			prev = curve;
		}

	}

	if (!select) return;

	for (int i = 0; i < path->len; i++) {
		point scaled_node = scale_to_screen(path->arr[i].x, path->arr[i].y,
				FIELD_RESCALED);

		if (path->selected == i)
			SDL_SetRenderDrawColor(window.renderer, 0x00, 0xFF, 0xFF, 0xFF);
		else
			SDL_SetRenderDrawColor(window.renderer, 0xFF, 0xFF, 0xFF, 0xFF);

		SDL_RenderDrawRect(window.renderer, &(SDL_Rect) {
					scaled_node.x - 5,
					scaled_node.y - 5,
					10,
					10,
				});
	}
}

void draw_robot(Field *field, point p) {
	const double size = .635;

	SDL_SetRenderDrawColor(window.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	point ll     = scale_to_screen(p.x - size / 2, p.y - size / 2, FIELD_RESCALED);
	point rr     = scale_to_screen(size, size, FIELD_RESCALED);
	point center = scale_to_screen(p.x, p.y, FIELD_RESCALED);

	SDL_RenderDrawPointF(window.renderer, center.x, center.y);
	SDL_RenderDrawRect(window.renderer, &(SDL_Rect) {
				.x = ll.x, .y = ll.y,
				.w = rr.x, .h = rr.y,
			});
}

void blit_screen(void) {
	SDL_RenderPresent(window.renderer);
}
