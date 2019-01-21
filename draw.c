#include <stdlib.h>
#include <SDL.h>
#include <string.h>
#include <stdio.h>

#include "resources_low.h"
#include "draw.h"
#include "player.h"
#include "enemies.h"
#include "map.h"
#include "bullets.h"
#include "sprite.h"
#include "text.h"
#include "crash.h"
#include "indicator.h"
#include "health.h"
#include "effects.h"
#include "cooldown.h"
#include "score.h"
#include "screen.h"
#include "curtain.h"

// Рисование игрока и врага - это рисование танка.
static void tank_draw(const tank *const _tank,
                      SDL_Texture *const _texture);

// Рисование индикатора.
static void indicator_draw(const float _x,
                           const float _y,
                           const float _cvalue,
                           const float _mvalue,
                           const Uint8 _r,
                           const Uint8 _g,
                           const Uint8 _b,
                           const Uint8 _a);

// Обрабатывает отрисовку игрока.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void player_draw(void)
{
    tank_draw(&player, texture_player);
}

// Рисует врагов.
// В случае ошибки показывается информацию о причине сбоя и крашит программу.
void enemies_draw(void)
{
    if (enemies_count == 0)
    {
        return;
    }

    for (size_t i = 0; i < MAX_ENEMIES; ++i)
    {
        if (enemies[i].active == 1)
        {
            tank_draw(&enemies[i], texture_enemy);
        }
    }
}

// Рисует карту - клетки заданного типа.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void map_draw(const cell_state _state)
{
    SDL_Rect rect;

    rect.w = SPRITE_SIZE;
    rect.h = SPRITE_SIZE;

    for (size_t x = 0; x < MAP_WIDTH; ++x)
    {
        for (size_t y = 0; y < MAP_HEIGHT; ++y)
        {
            SDL_Texture *texture = NULL;

            if (map[x][y] == _state)
            {
                switch(_state)
                {
                    case (CS_WALL):
                    {
                        texture = texture_wall;
                        break;
                    }
                    case (CS_TREE):
                    {
                        texture = texture_tree;
                        break;
                    }
                    case (CS_WATER):
                    {
                        texture = texture_water;
                        break;
                    }
                    case (CS_REPAIR):
                    {
                        texture = texture_repair;
                        break;
                    }
                    case (CS_EMPTY):
                    {

                        break;
                    }
                }
            }

            if (texture != NULL)
            {
                rect.x = x * SPRITE_SIZE;
                rect.y = y * SPRITE_SIZE;

                if (SDL_RenderCopyEx(renderer, texture, NULL, &rect, 0, NULL, SDL_FLIP_NONE) != 0)
                {
                    crash("map_draw(), не удалось отрисовать текстуру\nSDL_GetError() : %s",
                          SDL_GetError());
                }
            }
        }
    }
}

// Рисует все активные пули.
// В случае ошибки сообщает информацию о причине сбоя и крашит программу.
void bullets_draw(void)
{
    if (bullets_count == 0)
    {
        return;
    }

    for (size_t b = 0; b < MAX_BULLETS; ++b)
    {
        if (bullets[b].active == 1)
        {
            SDL_Rect rect;
            rect.w = SPRITE_SIZE;
            rect.h = SPRITE_SIZE;

            rect.x = bullets[b].x + 0.5f;
            rect.y = bullets[b].y + 0.5f;

            if (SDL_RenderCopy(renderer, texture_bullet, NULL, &rect) != 0)
            {
                crash("bullets_draw(), не удалось отрисовать пулю\nSDL_GetError() : %s",
                      SDL_GetError());
            }
        }
    }
}

// Рисует нуль-терменированную строку в виде однострочного текста.
// Нультерминатор ресуется, если вдруг для него существует текстура.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void text_draw(const char *const _text,
               const int _x,
               const int _y,
               const text_align _align)
{
    if (_text == NULL)
    {
        crash("text_draw(), _text == NULL");
    }

    const size_t text_length = strlen(_text);

    /*if (text_length == 0)
    {
        return;
    }*/

    if (text_length > MAX_TEXT_LENGTH)
    {
        crash("text_draw(), text_length > MAX_TEXT_LENGTH");
    }

    switch (_align)
    {
        // Вывод текста с левым выравниванием.
        case (TA_LEFT):
        {
            SDL_Rect rect;

            rect.x = _x;
            rect.y = _y;

            // Обходим символы строки слева направо (включая нультерминатор).
            for (size_t i = 0; i <= text_length; ++i)
            {
                const unsigned char glyph_index = _text[i];

                if (glyphs[glyph_index].texture != NULL)
                {
                    rect.h = glyphs[glyph_index].h;
                    rect.w = glyphs[glyph_index].w;
                    if (SDL_RenderCopy(renderer, glyphs[glyph_index].texture, NULL, &rect) != 0)
                    {
                        crash("text_draw(), не удалось отрисовать глиф при выводе текста с левым выравниванием\nSDL_GetError() : %s",
                              SDL_GetError());
                    }

                    rect.x += glyphs[glyph_index].w;
                }
            }
            break;
        }
        // Вывод текста с правым выравниванием.
        case (TA_RIGHT):
        {
            SDL_Rect rect;

            rect.x = _x;
            rect.y = _y;

            // Обходим символы строки справа налево (включая нультерминатор).
            for (size_t i = text_length; i <= text_length; --i)
            {
                const unsigned char glyph_index = _text[i];

                if (glyphs[glyph_index].texture != NULL)
                {
                    rect.h = glyphs[glyph_index].h;
                    rect.w = glyphs[glyph_index].w;

                    rect.x -= glyphs[glyph_index].w;

                    if (SDL_RenderCopy(renderer, glyphs[glyph_index].texture, NULL, &rect) != 0)
                    {
                        crash("text_draw(), не удалось отрисовать глиф при выводе текста с правым выравниванием\nSDL_GetError() : %s",
                              SDL_GetError());
                    }
                }
            }

            break;
        }
        case (TA_CENTER):
        {
            // Вычислим, сколько пикселей на экране займет строка (включая нультерминатор).
            int total_text_width = 0;
            for (size_t i = 0; i <= text_length; ++i)
            {
                const unsigned char glyph_index = _text[i];
                if (glyphs[glyph_index].texture != NULL)
                {
                    total_text_width += glyphs[glyph_index].w;
                }
            }

            const int offset = total_text_width / 2;

            SDL_Rect rect;

            rect.x = _x - offset;
            rect.y = _y;

            // Обходим символы строки слева направо (включая нультерминатор).
            for (size_t i = 0; i <= text_length; ++i)
            {
                const unsigned char glyph_index = _text[i];

                if (glyphs[glyph_index].texture != NULL)
                {
                    rect.h = glyphs[glyph_index].h;
                    rect.w = glyphs[glyph_index].w;
                    if (SDL_RenderCopy(renderer, glyphs[glyph_index].texture, NULL, &rect) != 0)
                    {
                        crash("text_draw(), не удалось отрисовать глиф при выводе текста с центральным выравниванием\nSDL_GetError() : %s",
                              SDL_GetError());
                    }

                    rect.x += glyphs[glyph_index].w;
                }
            }

            break;
        }
        default:
        {
            crash("Для отрисовки текста задано неизвестное выравнивание");
            break;
        }
    }
}

// Рисует индикаторы прочности и перезарядки игрока и врагов.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void indicators_draw(void)
{
    // Рисуем индикатор прочности игрока.
    indicator_draw(player.x,
                   player.y - 2 * INDICATOR_HEIGHT,
                   player.hp,
                   MAX_HEALTH,
                   0,
                   255,
                   0,
                   255);
    // Рисуем индикатор перезарядки игрока.
    indicator_draw(player.x,
                   player.y - INDICATOR_HEIGHT + 1,
                   SHOT_CD - player.c,
                   SHOT_CD,
                   0,
                   155,
                   255,
                   255);

    if (enemies_count > 0)
    {
        for (size_t e = 0; e < MAX_ENEMIES; ++e)
        {
            if (enemies[e].active == 1)
            {
                // Рисуем индикатор прочности врага.
                indicator_draw(enemies[e].x,
                               enemies[e].y - 2 * INDICATOR_HEIGHT,
                               enemies[e].hp,
                               MAX_HEALTH,
                               0,
                               255,
                               0,
                               255);
                // Рисуем индикатор перезарядки врага.
                indicator_draw(enemies[e].x,
                               enemies[e].y - INDICATOR_HEIGHT + 1,
                               SHOT_CD - enemies[e].c,
                               SHOT_CD,
                               0,
                               155,
                               255,
                               255);
            }
        }
    }
}

// Рисует танк.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void tank_draw(const tank *const _tank,
               SDL_Texture *const _texture)
{
    if (_tank == NULL)
    {
        crash("tank_draw(), _tank == NULL");
    }

    if (_texture == NULL)
    {
        crash("tank_draw(), _texture == NULL");
    }

    SDL_Rect rect;
    rect.x = _tank->x + 0.5f;
    rect.y = _tank->y + 0.5f;

    rect.h = SPRITE_SIZE;
    rect.w = SPRITE_SIZE;

    const double angle = _tank->d * 90;

    if (SDL_RenderCopyEx(renderer, _texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE) != 0)
    {
        crash("Не удалось отрисовать танк\nSDL_GetError() : %s",
              SDL_GetError());
    }
}

// Рисует индикатор заданного цвета в заданной точке.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void indicator_draw(const float _x,
                    const float _y,
                    const float _cvalue,
                    const float _mvalue,
                    const Uint8 _r,
                    const Uint8 _g,
                    const Uint8 _b,
                    const Uint8 _a)
{
    // Пытаемся задать цвет полоски индикатора.
    if (SDL_SetRenderDrawColor(renderer, _r, _g, _b, _a) != 0)
    {
        crash("indicator_draw(), не удалось задать цвет полоски индикатора\nSDL_getError() : %s",
              SDL_GetError());
    }

    if (_mvalue == 0.f)
    {
        crash("indicator_draw(), _mvalue == 0.f");
    }

    SDL_Rect rect;

    // Определяем размер и положение полоски.
    rect.w = INDICATOR_WIDTH * (_cvalue / _mvalue);
    rect.h = INDICATOR_HEIGHT;
    rect.x = _x + 0.5f;
    rect.y = _y + 0.5f;

    // Пытаемся нарисовать полоску.
    if (SDL_RenderFillRect(renderer, &rect) != 0)
    {
        crash("indicator_draw(), не удалось нарисовать полоску индикатора\nSDL_GetError() : %s",
              SDL_GetError());
    }
}

// Рисует эффекты.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void effects_draw(void)
{
    if (effects_count == 0)
    {
        return;
    }

    SDL_Rect rect;

    rect.w = SPRITE_SIZE;
    rect.h = SPRITE_SIZE;

    for (size_t e = 0; e < MAX_EFFECTS; ++e)
    {
        if (effects[e].active == 1)
        {
            rect.w = SPRITE_SIZE;
            rect.h = SPRITE_SIZE;

            rect.x = effects[e].x;
            rect.y = effects[e].y;

            if (SDL_SetTextureAlphaMod(effects[e].texture, effects[e].alpha + 0.5f) != 0)
            {
                crash("effects_draw(), SDL_SetTextureAlphaMod() != 0\nSDL_GetError() % s",
                      SDL_GetError());
            }

            if (SDL_RenderCopyEx(renderer,
                                 effects[e].texture,
                                 NULL,
                                 &rect,
                                 effects[e].angle,
                                 NULL,
                                 SDL_FLIP_NONE) != 0)
            {
                crash("effects_draw(), не удалось нарисовать эффект\nSDL_GetError() : %i",
                      SDL_GetError());
            }
        }
    }
}

// Отображает счет.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void score_draw(void)
{
    char text[1024];
    if (sprintf(text, "Очки: %Iu", score) < 0)
    {
        crash("score_draw(), sprintf() < 0\nerrno() : %i",
              errno);
    }

    text_draw(text, SCREEN_WIDTH / 2, SCREEN_HEIGHT - 44, TA_CENTER);
}

// Рисует приветствие.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void hello_draw(void)
{
    text_draw("Death Match Tanks",
              SCREEN_WIDTH / 2,
              SCREEN_HEIGHT / 2 - 11,
              TA_CENTER);

    char text[1024];

    if (sprintf(text, "Для начала боя нажмите \"Enter\", для выхода нажмите \"Escape\"") < 0)
    {
        crash("draw_hello(), sprintf() < 0\nerrno() : %i",
              errno);
    }
    text_draw(text,
              SCREEN_WIDTH / 2,
              SCREEN_HEIGHT - 64,
              TA_CENTER);
}

// Рисует занавес.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void curtain_draw(void)
{
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = SCREEN_WIDTH;
    rect.h = SCREEN_HEIGHT;

    if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, curtain) != 0)
    {
        crash("curtain_draw(), не удалось установить цвет заливки занавеса\nSDL_GetError() : %s",
              SDL_GetError());
    }

    if (SDL_RenderFillRect(renderer, &rect) != 0)
    {
        crash("curtain_draw(), не удалось отрисовать занавес\nSDL_GetError() : %s",
              SDL_GetError());
    }
}

// Рисует статистику.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void statistics_draw(void)
{
    char text[1024];
    if (sprintf(text, "Ваш результат: %Iu", score) < 0)
    {
        crash("statistics_draw(), sprintf() < 0\nerrno() : %i",
              errno);
    }
    text_draw(text, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 22, TA_CENTER);

    if (sprintf(text, "Для начала боя нажмите \"Enter\", для выхода нажмите \"Escape\"") < 0)
    {
        crash("statistics_draw(), sprintf() < 0\nerrno() : %i",
              errno);
    }
    text_draw(text,
              SCREEN_WIDTH / 2,
              SCREEN_HEIGHT - 64,
              TA_CENTER);
}
