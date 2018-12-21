/*
 * Модуль эффектов. Предоставляет сущности эффектов.
 *
 */
#ifndef EFFECTS_H
#define EFFECTS_H

#include <stdlib.h>
#include <SDL.h>

#define MAX_SCALE 4

#define MAX_EFFECTS 5000

typedef struct s_effect
{
    int active;

    float x;
    float y;

    float angle;
    float alpha;

    float d_angle;
    float d_alpha;

    SDL_Texture *texture;
} effect;

extern effect effects[MAX_EFFECTS];

extern size_t effects_count;

#endif
