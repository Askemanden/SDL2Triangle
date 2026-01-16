#include "../include/renderer.h"
#include "text.c"
#include "drawjob.c"
#include "drawjob_modifier.c"

static uint32_t buffer[WIDTH * HEIGHT];
static SDL_mutex *pixel_mutex = NULL;
static DrawJob *draw_queue = NULL;
static int draw_queue_length = 0;
static int draw_queue_capacity = 0;

int init_sdl(SDLContext *ctx)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return 1;

    ctx->window = SDL_CreateWindow(
        "Parallel Callback Renderer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT,
        SDL_WINDOW_SHOWN);

    if (!ctx->window)
        return 1;

    ctx->renderer = SDL_CreateRenderer(ctx->window, -1, SDL_RENDERER_ACCELERATED);
    if (!ctx->renderer)
        return 1;

    ctx->texture = SDL_CreateTexture(
        ctx->renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH, HEIGHT);

    if (!ctx->texture)
        return 1;

    pixel_mutex = SDL_CreateMutex();
    if (!pixel_mutex)
        return 1;

    return 0;
}

void shutdown_sdl(SDLContext *ctx)
{
    SDL_DestroyMutex(pixel_mutex);
    SDL_DestroyTexture(ctx->texture);
    SDL_DestroyRenderer(ctx->renderer);
    SDL_DestroyWindow(ctx->window);
    SDL_Quit();
}

void draw_pixel(int x, int y, uint32_t color)
{
    if ((unsigned)x >= WIDTH || (unsigned)y >= HEIGHT)
        return;

    buffer[y * WIDTH + x] = color;
}

void safe_draw_pixel(int x, int y, uint32_t color)
{
    if ((unsigned)x >= WIDTH || (unsigned)y >= HEIGHT)
        return;

    SDL_LockMutex(pixel_mutex);
    buffer[y * WIDTH + x] = color;
    SDL_UnlockMutex(pixel_mutex);
}

void update(SDLContext *ctx)
{
    SDL_UpdateTexture(ctx->texture, NULL, buffer, WIDTH * sizeof(uint32_t));
    SDL_RenderClear(ctx->renderer);
    SDL_RenderCopy(ctx->renderer, ctx->texture, NULL, NULL);
    SDL_RenderPresent(ctx->renderer);
}

void draw(DrawJob job)
{
    uint32_t (*callback)(int, int, void *) = job.callback;
    void *userdata = job.userdata;

#pragma omp parallel for collapse(2)
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            draw_pixel(x, y, callback(x, y, userdata));
}

void draw_bounded(DrawJob job)
{
    uint32_t (*callback)(int x, int y, void *) = job.callback;
    Recti area = job.area;
    void *userdata = job.userdata;
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
            uint32_t color = callback(x, y, userdata);
            draw_pixel(x, y, color);
        }
    }
}

void draw_multiple_bounded(DrawJob *jobs, int job_count)
{
#pragma omp parallel for
    for (int j = 0; j < job_count; j++)
    {
        Recti area = jobs[j].area;
        uint32_t (*callback)(int, int, void *) = jobs[j].callback;

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
                uint32_t color = callback(x, y, jobs[j].userdata);
                buffer[y * WIDTH + x] = color; // safe unless overlapping
            }
        }
    }
}

void draw_multiple_bounded_safe(DrawJob *jobs, int job_count)
{
    for (int j = 0; j < job_count; j++)
    {
        Recti area = jobs[j].area;
        uint32_t (*callback)(int, int, void *) = jobs[j].callback;
        void *userdata = jobs[j].userdata;

        int x0 = area.top_left.x;
        int y0 = area.top_left.y;
        int x1 = area.bottom_right.x;
        int y1 = area.bottom_right.y;

        if (x0 < 0)
            x0 = 0;
        if (y0 < 0)
            y0 = 0;
        if (x1 > WIDTH)
            x1 = WIDTH;
        if (y1 > HEIGHT)
            y1 = HEIGHT;

        // Parallelize the pixel loop, not the job loop
#pragma omp parallel for collapse(2)
        for (int y = y0; y < y1; y++)
        {
            for (int x = x0; x < x1; x++)
            {
                buffer[y * WIDTH + x] = callback(x, y, userdata);
            }
        }
    }
}

void enqueue_draw_job(DrawJob job)
{
    if (draw_queue_length == draw_queue_capacity)
    {
        draw_queue_capacity = draw_queue_capacity == 0 ? 16 : draw_queue_capacity * 2;
        draw_queue = realloc(draw_queue, draw_queue_capacity * sizeof(DrawJob));
    }

    draw_queue[draw_queue_length++] = job;
}

void process_queue()
{
    draw_multiple_bounded(draw_queue, draw_queue_length);
    draw_queue_length = 0;
}

void process_queue_safe()
{
    draw_multiple_bounded_safe(draw_queue, draw_queue_length);
    draw_queue_length = 0;
}
