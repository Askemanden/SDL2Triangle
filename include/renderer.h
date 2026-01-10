#pragma once
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <omp.h>

#define WIDTH 1000
#define HEIGHT 700

typedef struct
{
    int x;
    int y;
} Point;

typedef struct
{
    Point top_left;
    Point bottom_right;
} Rect;

typedef struct
{
    Rect area;
    uint32_t (*callback)(int x, int y);
} DrawJob;

typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
} SDLContext;

int init_sdl(SDLContext *ctx);

void shutdown_sdl(SDLContext *ctx);

void draw(uint32_t (*callback)(int x, int y));

void draw_bounded(uint32_t (*callback)(int x, int y), Rect area);

void draw_pixel(int x, int y, uint32_t color);

void update(SDLContext *ctx);

void draw_multiple_bounded(DrawJob *jobs, int job_count);

void enqueue_draw_job(DrawJob job);

