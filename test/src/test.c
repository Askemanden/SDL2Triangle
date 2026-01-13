#define SDL_MAIN_HANDLED

#include "../../include/renderer.h"
#include <time.h>

// Simple RGB helper
static uint32_t rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)r << 16) |
           ((uint32_t)g << 8) |
           (uint32_t)b;
}

/*
 * Callback used by draw() to fill entire screen with a gradient.
 */
static uint32_t gradient_callback(int x, int y, void *userdata)
{
    (void)userdata;

    float fx = (float)x / (float)WIDTH;
    float fy = (float)y / (float)HEIGHT;

    uint8_t r = (uint8_t)(fx * 255.0f);
    uint8_t g = (uint8_t)(fy * 255.0f);
    uint8_t b = 128;

    return rgb(r, g, b);
}

/*
 * Callback for a solid color, used in bounded draws and jobs.
 */
static uint32_t solid_red(int x, int y, void *userdata)
{
    (void)x; (void)y; (void)userdata;
    return rgb(255, 0, 0);
}

static uint32_t solid_green(int x, int y, void *userdata)
{
    (void)x; (void)y; (void)userdata;
    return rgb(0, 255, 0);
}

static uint32_t solid_blue(int x, int y, void *userdata)
{
    (void)x; (void)y; (void)userdata;
    return rgb(0, 0, 255);
}

/*
 * Helper to wait a bit while still pumping SDL events,
 * so the window stays responsive.
 */
static void wait_ms(unsigned int ms)
{
    Uint32 start = SDL_GetTicks();
    SDL_Event e;
    while (SDL_GetTicks() - start < ms)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                return;
        }
        SDL_Delay(10);
    }
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    SDLContext ctx;
    if (init_sdl(&ctx) != 0)
    {
        fprintf(stderr, "Failed to initialize SDL\n");
        return 1;
    }

    // 1) Test draw(): full-screen gradient
    DrawJob full_gradient = {
        .area = {{0,0},{WIDTH,HEIGHT}},
        .callback = gradient_callback,
        .userdata = NULL
    };

    draw(full_gradient);
    update(&ctx);
    printf("Showing full-screen gradient (draw)...\n");
    wait_ms(1000);

    // 2) Test draw_pixel() and safe_draw_pixel() directly
    int cx = WIDTH / 2;
    int cy = HEIGHT / 2;

    for (int dx = -20; dx <= 20; dx++)
    {
        draw_pixel(cx + dx, cy, rgb(255, 255, 255));
        draw_pixel(cx, cy + dx, rgb(255, 255, 255));
    }

    for (int y = cy - 10; y <= cy + 10; y++)
        for (int x = cx - 10; x <= cx + 10; x++)
            safe_draw_pixel(x, y, rgb(255, 255, 0));

    update(&ctx);
    printf("Showing draw_pixel and safe_draw_pixel result...\n");
    wait_ms(1000);

    // 3) Test draw_bounded(): draw a red rectangle in the top-left
    DrawJob red_rect = {
        .area = {{50,50},{300,200}},
        .callback = solid_red,
        .userdata = NULL
    };

    draw_bounded(red_rect);
    update(&ctx);
    printf("Showing draw_bounded (red rectangle)...\n");
    wait_ms(1000);

    // 4) Test draw_multiple_bounded() with non-overlapping jobs
    DrawJob jobs_non_overlap[3] = {
        { .area = {{0,HEIGHT/2},{WIDTH/3,HEIGHT}}, .callback = solid_red,   .userdata = NULL },
        { .area = {{WIDTH/3,HEIGHT/2},{2*WIDTH/3,HEIGHT}}, .callback = solid_green, .userdata = NULL },
        { .area = {{2*WIDTH/3,HEIGHT/2},{WIDTH,HEIGHT}}, .callback = solid_blue,  .userdata = NULL }
    };

    draw_multiple_bounded(jobs_non_overlap, 3);
    update(&ctx);
    printf("Showing draw_multiple_bounded with non-overlapping jobs...\n");
    wait_ms(1000);

    // 5) Test draw_multiple_bounded_safe() with overlapping jobs
    DrawJob jobs_overlap[3] = {
        { .area = {{100,100},{500,400}}, .callback = solid_red,   .userdata = NULL },
        { .area = {{300,200},{800,500}}, .callback = solid_green, .userdata = NULL },
        { .area = {{400,150},{900,550}}, .callback = solid_blue,  .userdata = NULL }
    };

    draw_multiple_bounded_safe(jobs_overlap, 3);
    update(&ctx);
    printf("Showing draw_multiple_bounded_safe with overlapping jobs...\n");
    wait_ms(1000);

    // 6) Test enqueue_draw_job() + process_queue()
    draw(full_gradient);

    DrawJob qjob1 = { .area = {{50,50},{350,300}}, .callback = solid_red,   .userdata = NULL };
    DrawJob qjob2 = { .area = {{200,150},{600,400}}, .callback = solid_green, .userdata = NULL };
    DrawJob qjob3 = { .area = {{650,100},{950,300}}, .callback = solid_blue,  .userdata = NULL };

    enqueue_draw_job(qjob1);
    enqueue_draw_job(qjob2);
    enqueue_draw_job(qjob3);

    process_queue();
    update(&ctx);
    printf("Showing process_queue (non-overlapping assumption)...\n");
    wait_ms(1000);

    // 7) Test enqueue_draw_job() + process_queue_safe()
    draw(full_gradient);

    DrawJob qjob4 = { .area = {{100,350},{600,650}}, .callback = solid_red,   .userdata = NULL };
    DrawJob qjob5 = { .area = {{200,400},{700,680}}, .callback = solid_green, .userdata = NULL };
    DrawJob qjob6 = { .area = {{300,450},{800,690}}, .callback = solid_blue,  .userdata = NULL };

    enqueue_draw_job(qjob4);
    enqueue_draw_job(qjob5);
    enqueue_draw_job(qjob6);

    process_queue_safe();
    update(&ctx);
    printf("Showing process_queue_safe (overlapping jobs, last wins)...\n");
    wait_ms(3000);

    shutdown_sdl(&ctx);
    return 0;
}
