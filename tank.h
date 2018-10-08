/*
 * Модуль танка. Предоставляет описание внутреннего устройства танка.
 *
 */

#ifndef TANK_H
#define TANK_H

#include "direction.h"

typedef struct s_tank
{
    // Активность, имеет значение только для танков врагов.
    int active;

    int hp;
    float speed;
    direction d;

    // Контроль перезарядки оружия.
    float c;

    float x;
    float y;

    float tx;
    float ty;

    // Прошлая клетка, имеет значение только для танков врагов.
    float ox;
    float oy;



} tank;

#endif
