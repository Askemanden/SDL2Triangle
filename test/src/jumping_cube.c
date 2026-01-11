#include "../../include/renderer.h"
#include <stdint.h>

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
static uint32_t gradient_callback(int x, int y)
{
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
static uint32_t solid_red(int x, int y)
{
    (void)x;
    (void)y;
    return rgb(255, 0, 0);
}

static uint32_t solid_green(int x, int y)
{
    (void)x;
    (void)y;
    return rgb(0, 255, 0);
}

static uint32_t solid_blue(int x, int y)
{
    (void)x;
    (void)y;
    return rgb(0, 0, 255);
}

int main(int argc, char *argv[])
{
    SDLContext ctx = {0};

    if (init_sdl(&ctx))
        return 1;

    // Basic event loop
    SDL_Event e;
    int running = 1;

    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                running = 0;
        }
        SDL_Delay(10);
    }

    shutdown_sdl(&ctx);
    return 0;
}