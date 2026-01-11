#pragma once
#include "drawjob.h"
/// @brief A rectangular bitmap of RGB values
/// @param width Width of the bitmap in pixels
/// @param height Height of the bitmap in pixels
/// @param bitmap_argb List of all items in the bitmap. Each pixel is represented by a uint32_t RGB color 
/// (Technically ARGB but alpha is not supported)
typedef struct Bitmap
{
    int width;
    int height;
    uint32_t *bitmap_argb;
} Bitmap;

/// @brief Creates a draw job that can be enqueued to draw a bitmap at the given position.
/// Recommend reusing the same draw job for redrawing the same bitmap at the same position multiple times
/// @param bitmap The bitmap to draw
/// @param x_pos x-coordinate to draw the bitmap at
/// @param y_pos y-coordinate to draw the bitmap at
/// @return A draw job that can be enqueued to draw the bitmap at the given coordinates
DrawJob create_bitmap_draw_job(Bitmap bitmap, int x_pos, int y_pos);

/// @brief Scales a bitmap by the scale.
/// Will round width and height down in case of floating point values arising from scaling. This will result in non-smooth scaling.
/// @param bitmap
/// @param scale
/// @return
Bitmap scale_bitmap(Bitmap bitmap, double scale);