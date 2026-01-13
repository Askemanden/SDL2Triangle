#include "../../include/renderer.h"

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
static uint32_t gradient_callback(int x, int y, void *_)
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
static uint32_t solid_blue(int x, int y, void *_)
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

    Rectf square = {.top_left = {.x = 10, .y = 10}, .bottom_right = {.x = 100, .y = 100}};
    double velocity[] = {120, 70};

    DrawJob square_job = {.area = Rectf_to_i(square), .callback = solid_blue};

    // Basic event loop
    SDL_Event e;
    int running = 1;

    Uint32 last_fps_time = SDL_GetTicks();
    int frame_count = 0;
    uint32_t last_frame_time = 0;
    double dt = 0;

    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                running = 0;
        }

        frame_count++;
        Uint32 now = SDL_GetTicks();
        if (now - last_fps_time >= 1000)
        {
            float fps = (float)frame_count * 1000.0f / (now - last_fps_time);
            printf("FPS: %.2f\n", fps);

            frame_count = 0;
            last_fps_time = now;
            printf("DT: %.10f\n", dt);
        }

        dt = (now - last_frame_time) / 1000.0f;

        last_frame_time = now;

        // Move square
        square.top_left.x += velocity[0] * dt;
        square.top_left.y += velocity[1] * dt;
        square.bottom_right.x += velocity[0] * dt;
        square.bottom_right.y += velocity[1] * dt;

        // Bounce logic
        if (square.bottom_right.x >= WIDTH && velocity[0] > 0)
            velocity[0] = -velocity[0];
        else if (square.top_left.x <= 0 && velocity[0] < 0)
            velocity[0] = -velocity[0];

        if (square.bottom_right.y >= HEIGHT && velocity[1] > 0)
            velocity[1] = -velocity[1];
        else if (square.top_left.y <= 0 && velocity[1] < 0)
            velocity[1] = -velocity[1];

        square_job.area = Rectf_to_i(square);

        // Draw background
        draw((DrawJob){.callback = gradient_callback});

        // Draw square
        enqueue_draw_job(square_job);
        process_queue();

        update(&ctx);
    }

    shutdown_sdl(&ctx);
    return 0;
}
