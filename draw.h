/*
 * Модуль рисования. Предоставляет функции отрисовки игрока, врагов, пуль, карты и пр.
 *
 */

#ifndef DRAW_H
#define DRAW_H

#include "cell.h"
#include "text.h"

void player_draw(void);

void enemies_draw(void);

void map_draw(const cell_state _state);

void bullets_draw(void);

void text_draw(const char *const _text,
               const int _x,
               const int _y,
               const text_align _align);

void indicators_draw(void);

void effects_draw(void);

void score_draw(void);

void hello_draw(void);

void curtain_draw(void);

void statistics_draw(void);

#endif
