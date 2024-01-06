#pragma once

#include "../include/SDL.h"
#include "../include/SDL_events.h"
#include "../include/SDL_pixels.h"
#include "../include/SDL_surface.h"
#include "../include/SDL_video.h"
#include "../game/motion.h"
#include "../game/planner.h"

#define SCREEN_WIDTH  1000
#define SCREEN_HEIGHT 500

#define SDL_FLAGS    (SDL_INIT_VIDEO)
#define RENDER_FLAGS (SDL_RENDERER_ACCELERATED)
#define IMG_FLAGS    (IMG_INIT_PNG | IMG_INIT_JPG)

// SDL Metadata
typedef struct {
	SDL_Window   *window;
	SDL_Surface  *surface;
	SDL_Renderer *renderer;
	SDL_Texture  *texture;
} Window;

int init_gfx(bool funny);

#define DRAW_PATH(path, field)        draw_path(path, field, false, false)
#define DRAW_PATH_BOLD(path, field)   draw_path(path, field, true,  false)
#define DRAW_PATH_BLUE(path, field)    draw_path(path, field, false, true)
#define DRAW_PATH_SELECT(path, field) draw_path(path, field, true,  true)

void draw_field(Field *field);
void draw_path(MotionPath *path, Field *field, bool bold, bool select);
void draw_bezier(BezierPath *path, Field *field, bool bold, bool select);

void draw_robot(Field *field, point p);

void blit_screen(void);
