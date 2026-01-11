#pragma once
#include <stdint.h>

typedef struct Point
{
    int x;
    int y;
} Point;

typedef struct Rect
{
    Point top_left;
    Point bottom_right;
} Rect;

typedef struct DrawJob
{
    Rect area;
    uint32_t (*callback)(int x, int y);
} DrawJob;