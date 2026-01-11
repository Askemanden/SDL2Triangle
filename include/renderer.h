#pragma once
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <omp.h>

#include "../src/text.h"
#include "../src/drawjob.h"

#define WIDTH 1000
#define HEIGHT 700

typedef struct SDLContext
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
} SDLContext;

/// @brief SDL initialization
/// @param ctx empty SDLContext object to populate
/// @return return 0 for successful initialization and 1 for failure
int init_sdl(SDLContext *ctx);

/// @brief Shuts down SDLContext
/// @param ctx SDLContext to shut down
void shutdown_sdl(SDLContext *ctx);

/// @brief Draws to the buffer using callback as a function of pixel coordinates for pixel color values
/// @param callback Callback to create pixel color values based on x and y coordinate.
/// Must be a pure function to ensure multi threaded drawing will work
void draw(uint32_t (*callback)(int x, int y));

/// @brief Limited area parallel buffer fill using a callback.
/// Recommend using draw_multiple_bounded with a custom queue or enqueue_draw_job
/// with process_queue
/// @param callback function to be called whose return is the value for every pixel in the area, given the pixels x and y coordinate as input
/// @param area bounding area
void draw_bounded(uint32_t (*callback)(int x, int y), Rect area);

/// @brief Not thread safe pixel drawing function
/// @param x pixel x-coordinate
/// @param y pixel y-coordinate
/// @param color pixel color
void draw_pixel(int x, int y, uint32_t color);

/// @brief Thread safe pixel drawing function
/// @param x pixel x-coordinate
/// @param y pixel y-coordinate
/// @param color pixel color
void safe_draw_pixel(int x, int y, uint32_t color);

/// @brief buffer to SDL texture and draw it
/// @param ctx SDLContext to update
void update(SDLContext *ctx);

/// @brief Draws multiple draw jobs in parallel. Recommend using inbuilt draw queue with enqueue_draw_job
/// unless multiple queues must be maintaned seperately. Draw jobs should not overlap  in area
/// @param jobs list of jobs to complete.
/// @param job_count length of `jobs`
void draw_multiple_bounded(DrawJob *jobs, int job_count);

/// @brief Same as draw_multiple_bounded, but allows overlapping areas. Later areas will override earlyer ones upon overlap
/// @param jobs list of jobs to complete.
/// @param job_count length of `jobs`
void draw_multiple_bounded_safe(DrawJob *jobs, int job_count);

/// @brief Add draw job to the draw queue
/// @param job draw job to add to the queue
void enqueue_draw_job(DrawJob job);

/// @brief calls draw_multiple_bounded on the inbuilt draw_queue
void process_queue();

/// @brief calls draw_multiple_bounded_safe on the inbuilt draw_queue
void process_queue_safe();