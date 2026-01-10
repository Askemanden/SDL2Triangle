#include "renderer.h"
#include <stdint.h>
#include <stdio.h>

// Simple callback: returns a color based on pixel position
static uint32_t gradient(int x, int y)
{
    uint8_t r = (x * 255) / WIDTH;
    uint8_t g = (y * 255) / HEIGHT;
    uint8_t b = ((x + y) * 255) / (WIDTH + HEIGHT);
    return (0xFFu << 24) | (r << 16) | (g << 8) | b;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    SDLContext ctx;

    if (!init_sdl(&ctx))
        return 1;

    SDL_Event e;
    int running = 1;

    uint64_t total_frames = 0;
    uint32_t start_time = SDL_GetTicks();

    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                running = 0;
        }

        // Render a frame
        draw(gradient);
        update(&ctx);

        total_frames++;
    }

    uint32_t end_time = SDL_GetTicks();
    double elapsed_seconds = (end_time - start_time) / 1000.0;

    double average_fps = 0.0;
    if (elapsed_seconds > 0.0)
        average_fps = total_frames / elapsed_seconds;

    // Write to log.txt
    FILE *log = fopen("log.txt", "w");
    if (log)
    {
        fprintf(log, "Average FPS: %.2f\n", average_fps);
        fclose(log);
    }

    shutdown_sdl(&ctx);
    return 0;
}
