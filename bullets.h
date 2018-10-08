/*
 * Модуль пуль. Предоставляет пул сущностей пуль.
 *
 */

#ifndef BULLETS_H
#define BULLETS_H

#include <stdlib.h>
#include "direction.h"
#include "owner.h"

#define MAX_BULLETS 1024

typedef struct s_bullet
{
    int active;

    float x;
    float y;

    direction d;

    owner owner;

} bullet;

extern size_t bullets_count;

extern bullet bullets[MAX_BULLETS];

#endif
