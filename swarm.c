#include <jwb.h>
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define jwb_num_t float

#define PIXEL_SCALE 10
#define CELL_SIZE 10
#define NUM_ENTS 1000

jwb_num_t frand(void)
{
	return (jwb_num_t)rand() / RAND_MAX;
}

void Initialize(void);
SDL_Window *CreateWindow(void);
void Simulate(SDL_Window *window);
void Quit(SDL_Window *window);
int main(int argc, char *argv[])
{
	SDL_Window *window;

	Initialize();
	window = CreateWindow();
	Simulate(window);
	Quit(window);

}

#define die(...) ( die_fun(__LINE__, __VA_ARGS__) )
static _Noreturn void die_fun(int line, const char *fmt, ...)
	__attribute__(( format(printf, 2, 3) ));

void Initialize(void)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		die("SDL init failed: %s\n", SDL_GetError());
	}
	srand(time(NULL));
}
SDL_Window *CreateWindow(void)
{
	SDL_Window *window;
	window = SDL_CreateWindow("Swarming",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		0,
		0,
		SDL_WINDOW_SHOWN);
	if (!window
	 || SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP)) {
		die("SDL window creation failed: %s\n", SDL_GetError());
	}
	return window;
}
void DrawCircle(SDL_Renderer *renderer, jwb_num_t x, jwb_num_t y, jwb_num_t radius)
{
	static const struct jwb_vect pts[] = {
#		include "circle-points.h"
	};
	static SDL_Point linebuf[sizeof(pts) / sizeof(*pts)];
	x = (x - CELL_SIZE) * PIXEL_SCALE;
	y = (y - CELL_SIZE) * PIXEL_SCALE;
	radius *= PIXEL_SCALE;
	size_t i;
	for (i = 0; i < sizeof(pts) / sizeof(*pts); ++i) {
		linebuf[i].x = x + pts[i].x * radius;
		linebuf[i].y = y + pts[i].y * radius;
	}
	SDL_RenderDrawLines(renderer, linebuf, i);
}
static void draw_ent(jwb_world_t *world, jwb_ehandle_t ent,
	SDL_Renderer *renderer)
{
	struct jwb_vect pos, off;
	jwb_num_t radius;
	jwb_world_get_pos_unck(world, ent, &pos);
	jwb_world_get_offset(world, &off);
	radius = jwb_world_get_radius_unck(world, ent);
	DrawCircle(renderer, pos.x - off.x, pos.y - off.y, radius);
}
static void draw_ents(jwb_world_t *world, SDL_Renderer *renderer)
{
	jwb_ehandle_t e;
	for (e = jwb_world_first(world); e >= 0; e = jwb_world_next(world, e)) {
		draw_ent(world, e, renderer);
	}
}
void collide(
	jwb_world_t *world,
	jwb_ehandle_t e1,
	jwb_ehandle_t e2,
	struct jwb_hit_info *info)
{
	jwb_num_t mass1, mass2;
	mass1 = jwb_world_get_mass_unck(world, e1);
	mass2 = jwb_world_get_mass_unck(world, e2);
	jwb_inelastic_collision(world, e1, e2, info);
	info->rel.x /= info->dist * 15.;
	info->rel.y /= info->dist * 15.;
	jwb_world_accelerate(world, e1, &info->rel);
	info->rel.x *= -1;
	info->rel.y *= -1;
	jwb_world_accelerate(world, e2, &info->rel);
}
bool should_quit(const SDL_Event *event)
{
	switch (event->type) {
	case SDL_QUIT:
		return true;
	case SDL_KEYUP:
		return event->key.keysym.sym == SDLK_q;
	default:
		return false;
	}
}
void Simulate(SDL_Window *window)
{
	int pixw, pixh;
	jwb_world_t *world = malloc(sizeof(*world));
	struct jwb_world_init alloc_info = {
		.flags = 0,
		.cell_size = CELL_SIZE,
		.ent_buf_size = 0,
		.ent_extra = 0,
		.ent_buf = NULL,
		.cell_buf = NULL
	};
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_Event event;
	SDL_GetWindowSize(window, &pixw, &pixh);
	alloc_info.width = pixw / CELL_SIZE / PIXEL_SCALE + 2;
	alloc_info.height = pixh / CELL_SIZE / PIXEL_SCALE + 2;
	jwb_world_alloc(world, &alloc_info);
	jwb_world_on_hit(world, collide);
	for (int i = 0; i < NUM_ENTS; ++i) {
		jwb_ehandle_t ent;
		struct jwb_vect pos, vel;
		jwb_num_t radius, mass;
		pos.x = frand() + 20;
		pos.y = frand() + 20;
		vel.x = frand() - 0.5;
		vel.y = frand() - 0.5;
		radius = 0.5 + frand();
		mass = radius * radius;
		ent = jwb_world_add_ent(world, &pos, &vel, mass, radius);
		if (ent < 0) {
			die("JWB Error: %s", jwb_errmsg(ent));
		}
	}
	while (1) {
		if (SDL_PollEvent(&event) && should_quit(&event)) {
			break;
		}
		jwb_world_step(world);
		jwb_world_apply_friction(world, 0.001);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		draw_ents(world, renderer);
		SDL_RenderPresent(renderer);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_Delay(40);
	}
	jwb_world_destroy(world);
	free(world);
}
void Quit(SDL_Window *window)
{
	SDL_DestroyWindow(window);
	SDL_Quit();
}

static _Noreturn void die_fun(int line, const char *fmt, ...)
{
	va_list args;
	fprintf(stderr, "Died on line %d: ", line);
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fflush(stderr);
	exit(1);
}
