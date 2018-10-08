/*
 * Модуль глифа. Предоставляет описание внутреннего устройства глифа.
 *
 */

#ifndef GLYPH_H
#define GLYPH_H

#include "SDL.h"

typedef struct s_glyph
{
    int h, w;
    SDL_Texture *texture;
} glyph;

#endif
