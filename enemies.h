/*
 * Модуль врагов. Предоставляет пул сущностей врагов.
 *
 */

#ifndef ENEMIES_H
#define ENEMIES_H

#include <stdlib.h>
#include "tank.h"

#define MAX_ENEMIES 128

extern size_t enemies_count;

extern tank enemies[MAX_ENEMIES];

#endif
