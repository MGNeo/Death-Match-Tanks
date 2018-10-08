/*
 * Модуль обработки. Предоставляет функции обработки, сброса и добавления.
 *
 */
#ifndef PROCESSING_H
#define PROCESSING_H

#include <SDL.h>
#include "direction.h"

void processing_player(const Uint8 *const _keys, const float _dt);
void processing_enemies(const float _dt);
void processing_bullets(const float _dt);
void processing_effects(const float _dt);
void processing_curtain(const float _dt);

void reset_player(void);
void reset_enemies(void);
void reset_bullets(void);
void reset_map(void);
void reset_effects(void);
void reset_curtain(void);

void reset_score(void);

void add_enemy(void);

int player_is_dead(void);

int curtain_is_max(void);

#endif
