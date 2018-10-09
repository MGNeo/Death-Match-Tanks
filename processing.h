/*
 * Модуль обработки. Предоставляет функции обработки, сброса и добавления.
 *
 */
#ifndef PROCESSING_H
#define PROCESSING_H

#include <SDL.h>
#include "direction.h"

void player_processing(const Uint8 *const _keys, const float _dt);
void enemies_processing(const float _dt);
void bullets_processing(const float _dt);
void effects_processing(const float _dt);
void curtain_processing(const float _dt);

void player_reset(void);
void enemies_reset(void);
void bullets_reset(void);
void map_reset(void);
void effects_reset(void);
void curtain_reset(void);
void score_reset(void);

void enemy_add(void);

int player_is_dead(void);

int curtain_is_max(void);

#endif
