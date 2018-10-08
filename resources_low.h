/*
 * Низкоуровневый модуль ресурсов. Предоставляет любому модулю прямой доступ ко всем
 * ресурсам игры.
 *
 */

#ifndef RESOURCES_LOW_H
#define RESOURCES_LOW_H

#include "SDL.h"
#include "SDL_mixer.h"
#include "glyph.h"
#include "map.h"

extern SDL_Window *window;

extern SDL_Renderer *renderer;

extern glyph glyphs[];

extern SDL_Texture *texture_player,
                   *texture_enemy,
                   *texture_wall,
                   *texture_tree,
                   *texture_water,
                   *texture_repair,
                   *texture_bullet,
                   *texture_flush,
                   *texture_smoke,
                   *texture_capture;


extern Mix_Chunk *sound_shot,
                 *sound_hit_to_wall,
                 *sound_hit_to_enemy,
                 *sound_hit_to_player,
                 *sound_enemy_explode,
                 *sound_player_explode,
                 *sound_repair;

extern cell_state map_template[MAP_WIDTH][MAP_HEIGHT];

#endif
