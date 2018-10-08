#include "resources_low.h"

// Окно.
SDL_Window *window = NULL;

// Рендерер.
SDL_Renderer *renderer = NULL;

// Глифы букв.
glyph glyphs[256] = {{0, 0, NULL}};

// Текстуры.
SDL_Texture *texture_player = NULL,
            *texture_enemy = NULL,
            *texture_wall = NULL,
            *texture_tree = NULL,
            *texture_water = NULL,
            *texture_repair = NULL,
            *texture_bullet = NULL,
            *texture_flush = NULL,
            *texture_smoke = NULL,
            *texture_capture = NULL;

// Звуки.
Mix_Chunk *sound_shot = NULL,
          *sound_hit_to_wall = NULL,
          *sound_hit_to_enemy = NULL,
          *sound_hit_to_player = NULL,
          *sound_enemy_explode = NULL,
          *sound_player_explode = NULL,
          *sound_repair = NULL;

// Шаблон карты (не меняется после загрузки).
cell_state map_template[MAP_WIDTH][MAP_HEIGHT];

// Подумать над организацией структуры карты так, чтобы обход шел построчно и
// кэш процессора использовался наиболее эффективно.
