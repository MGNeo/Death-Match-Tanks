/*
 * Модуль рисования. Предоставляет функции отрисовки игрока, врагов, пуль, карты и пр.
 *
 */

#ifndef DRAW_H
#define DRAW_H

#include "cell.h"
#include "text.h"

void draw_player(void);

void draw_enemies(void);

void draw_map(const cell_state _state);

void draw_bullets(void);

void draw_text(const char *const _text,
               const int _x,
               const int _y,
               const text_align _align);

void draw_indicators(void);

void draw_effects(void);

void draw_score(void);

void draw_hello(void);

void draw_curtain(void);

void draw_statistics(void);

#endif
