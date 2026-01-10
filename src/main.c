#include "../include/renderer.h"

#include "bitmaps.c"

static uint32_t buffer[WIDTH * HEIGHT];
static SDL_mutex *pixel_mutex = NULL;
static DrawJob *draw_queue = NULL;
static int draw_queue_length = 0;
static int draw_queue_capacity = 0;

// SDL Initialization
// -
int init_sdl(SDLContext *ctx)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return 0;

    ctx->window = SDL_CreateWindow(
        "Parallel Callback Renderer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT,
        SDL_WINDOW_SHOWN);

    if (!ctx->window)
        return 0;

    ctx->renderer = SDL_CreateRenderer(ctx->window, -1, SDL_RENDERER_ACCELERATED);
    if (!ctx->renderer)
        return 0;

    ctx->texture = SDL_CreateTexture(
        ctx->renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH, HEIGHT);

    if (!ctx->texture)
        return 0;

    pixel_mutex = SDL_CreateMutex();
    if (!pixel_mutex)
        return 0;

    return 1;
}

// ------------------------------------------------------------
// SDL Cleanup
// ------------------------------------------------------------
void shutdown_sdl(SDLContext *ctx)
{
    SDL_DestroyMutex(pixel_mutex);
    SDL_DestroyTexture(ctx->texture);
    SDL_DestroyRenderer(ctx->renderer);
    SDL_DestroyWindow(ctx->window);
    SDL_Quit();
}

/// @brief Not thread safe pixel drawing function
/// @param x pixel x-coordinate
/// @param y pixel y-coordinate
/// @param color pixel color
void draw_pixel(int x, int y, uint32_t color)
{
    if ((unsigned)x >= WIDTH || (unsigned)y >= HEIGHT)
        return;

    buffer[y * WIDTH + x] = color;
}

/// @brief Thread safe pixel drawing function
/// @param x pixel x-coordinate
/// @param y pixel y-coordinate
/// @param color pixel color
void safe_draw_pixel(int x, int y, uint32_t color)
{
    if ((unsigned)x >= WIDTH || (unsigned)y >= HEIGHT)
        return;

    SDL_LockMutex(pixel_mutex);
    buffer[y * WIDTH + x] = color;
    SDL_UnlockMutex(pixel_mutex);
}

/// @brief buffer to SDL texture and draw it
/// @param ctx SDLContext to update
void update(SDLContext *ctx)
{
    SDL_UpdateTexture(ctx->texture, NULL, buffer, WIDTH * sizeof(uint32_t));
    SDL_RenderClear(ctx->renderer);
    SDL_RenderCopy(ctx->renderer, ctx->texture, NULL, NULL);
    SDL_RenderPresent(ctx->renderer);
}

// ------------------------------------------------------------
// Parallel buffer fill using a callback
// ------------------------------------------------------------
void draw(uint32_t (*callback)(int x, int y))
{
#pragma omp parallel for collapse(2)
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            draw_pixel(x, y, callback(x, y));
}

/// @brief Limited area parallel buffer fill using a callback.
/// Recommend using draw_multiple_bounded with a custom queue or enqueue_draw_job
/// with process_queue
/// @param callback function to be called whose return is the value for every pixel in the area, given the pixels x and y coordinate as input
/// @param area bounding area
void draw_bounded(uint32_t (*callback)(int x, int y), Rect area)
{
    int x0 = area.top_left.x;
    int y0 = area.top_left.y;
    int x1 = area.bottom_right.x;
    int y1 = area.bottom_right.y;

    // Clamp to framebuffer bounds
    if (x0 < 0)
        x0 = 0;
    if (y0 < 0)
        y0 = 0;
    if (x1 > WIDTH)
        x1 = WIDTH;
    if (y1 > HEIGHT)
        y1 = HEIGHT;

#pragma omp parallel for collapse(2)
    for (int y = y0; y < y1; y++)
    {
        for (int x = x0; x < x1; x++)
        {
            uint32_t color = callback(x, y);
            draw_pixel(x, y, color);
        }
    }
}

/// @brief Draws multiple draw jobs in parallel. Recommend using inbuilt draw queue with enqueue_draw_job
/// unless multiple queues must be maintaned seperately
/// @param jobs list of jobs to complete. Will be completed in order from index 0 to `job_count`-1
/// @param job_count length of `jobs`
void draw_multiple_bounded(DrawJob *jobs, int job_count)
{
#pragma omp parallel for
    for (int j = 0; j < job_count; j++)
    {
        Rect area = jobs[j].area;
        uint32_t (*callback)(int, int) = jobs[j].callback;

        int x0 = area.top_left.x;
        int y0 = area.top_left.y;
        int x1 = area.bottom_right.x;
        int y1 = area.bottom_right.y;

        // Clamp
        if (x0 < 0)
            x0 = 0;
        if (y0 < 0)
            y0 = 0;
        if (x1 > WIDTH)
            x1 = WIDTH;
        if (y1 > HEIGHT)
            y1 = HEIGHT;

        for (int y = y0; y < y1; y++)
        {
            for (int x = x0; x < x1; x++)
            {
                uint32_t color = callback(x, y);
                buffer[y * WIDTH + x] = color; // safe unless overlapping
            }
        }
    }
}

/// @brief Add draw job to the draw queue
/// @param job draw job to add to the queue
void enqueue_draw_job(DrawJob job)
{
    if (draw_queue_length == draw_queue_capacity)
    {
        draw_queue_capacity = draw_queue_capacity == 0 ? 16 : draw_queue_capacity * 2;
        draw_queue = realloc(draw_queue, draw_queue_capacity * sizeof(DrawJob));
    }

    draw_queue[draw_queue_length++] = job;
}

/// @brief calls draw_multiple_bounded on the inbuilt draw_queue
void process_queue()
{
    draw_multiple_bounded(draw_queue, draw_queue_length);
}
