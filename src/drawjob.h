#pragma once
#include <stdint.h>

typedef struct Pointi
{
    int x;
    int y;
} Pointi;

typedef struct Recti
{
    Pointi top_left;
    Pointi bottom_right;
} Recti;

typedef struct Pointf
{
    double x;
    double y;
} Pointf;

typedef struct Rectf
{
    Pointf top_left;
    Pointf bottom_right;
} Rectf;

typedef struct DrawJob
{
    Recti area;
    uint32_t (*callback)(int x, int y);
} DrawJob;

Recti Rectf_to_i(Rectf rectf);