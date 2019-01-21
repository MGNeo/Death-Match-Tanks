#include <stdlib.h>
#include <stdio.h>
#include <SDL_ttf.h>
#include <windows.h>
#include <string.h>
#include <locale.h>

#include "screen.h"
#include "resources_medium.h"
#include "resources_low.h"
#include "crash.h"

// Загружает текстуру из файла и записывает ее адрес в заданное расположение.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
static void texture_load(const char *const _file_name,
                         SDL_Texture **const _texture)
{
    if (_file_name == NULL)
    {
        crash("texture_load(), _file_name == NULL");
    }

    if (strlen(_file_name) == 0)
    {
        crash("texture_load(), strlen(_file_name) == 0");
    }

    if (_texture == NULL)
    {
        crash("texture_load(), _texture == NULL");
    }

    SDL_Surface *h_surface = SDL_LoadBMP(_file_name);
    if (h_surface == NULL)
    {
        crash("texture_load(), не удалось создать поверхность на основе файла: %s\nSDL_GetError() : %s",
              _file_name,
              SDL_GetError());
    }

    SDL_Texture *h_texture = SDL_CreateTextureFromSurface(renderer, h_surface);
    if (h_texture == NULL)
    {
        crash("texture_load(), не удалось создать текстуру на основе поверхности\nSDL_GetError() : %s",
              SDL_GetError());
    }

    SDL_FreeSurface(h_surface);

    *_texture = h_texture;
}

// Загружает звук из файла.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
static void sound_load(const char *const _file_name,
                       Mix_Chunk **const _sound)
{
    if (_file_name == NULL)
    {
        crash("sound_load(), _file_name == NULL");
    }
    if (_sound == NULL)
    {
        crash("sound_load(), _sound == NULL");
    }

    *_sound = Mix_LoadWAV(_file_name);

    if (*_sound == NULL)
    {
        crash("sound_load(), не удалось загрузить звук\nMix_GetError() : %s",
              Mix_GetError());
    }
}

// Инициализирует все системы.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void system_init(void)
{
    // Защита от повторного вызова.
    static size_t again = 0;
    if (again++ != 0)
    {
        crash("system_init(), произошла попытка повторной инициализации всех систем");
    }

    // Инициализация SDL.
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        crash("system_init(), не удалось инициализировать SDL\nSDL_Init() != 0\nSDL_GetError() : %s",
              SDL_GetError());
    }

    // Инициализация SDL_ttf.
    if (TTF_Init() != 0)
    {
        crash("system_init(), не удалось инициализировать SDL_ttf\nTTF_Init() != 0\nTTF_GetError() : %s",
              TTF_GetError());
    }

    // Инициализация SDL_mixer.
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 1, 128) != 0)
    {
        crash("system_init(), не удалось инициализировать SDL_mixer\nMix_OpenAudio() != 0\nMix_GetError() : %s",
              Mix_GetError());
    }

    // Установим количество смешиваемых каналов.
    Mix_AllocateChannels(128);
    // Создание окна.
    window = SDL_CreateWindow("Death Match Tanks",
                              0,
                              0,
                              SCREEN_WIDTH,
                              SCREEN_HEIGHT,
                              SDL_WINDOW_FULLSCREEN);
    if (window == NULL)
    {
        crash("system_init(), не удалось создать окно\nSDL_GetError() : %s",
              SDL_GetError());
    }
    // Создание рендерера.
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);// SDL_RENDERER_PRESENTVSYNCd
    if (renderer == NULL)
    {
        crash("system_init(), не удалось создать рендер\nSDL_GetError() : %s",
              SDL_GetError());
    }

    // Задаем настройки смешивания.
    if (SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND) != 0)
    {
        crash("system_init(), не удалось установить рендеру режим смешивания\nSDLGetError() : %s",
              SDL_GetError());
    }
}

// Загружает шрифт.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void font_load(void)
{
    // Защита от повторного вызова.
    static size_t again = 0;
    if (again++ != 0)
    {
        crash("font_load(), произошла попытка повторной загрузки шрифта");
    }

    // Загрузим шрифт.
    TTF_Font *h_font = TTF_OpenFont("font/font.ttf", 22);
    if (h_font == NULL)
    {
        crash("font_load(), не удалось загрузить шрифт\nTTF_GetError() : %s",
              TTF_GetError());
    }

    // Заполним глифы текстурами символов.
    SDL_Color h_color = {255, 255, 255, 255};
    for (size_t i = 0; i < 256; ++i)
    {
        const char ansi_char = (char)i;
        // Защита от переполнения, если символ представлен суррогатной парой.
        Uint16 unicode_char[2] = {0};
        // Преобразуем символ из текущей кодировки (cp1251) в юникод.
        const int r_code = MultiByteToWideChar(1251,
                                               0,
                                               &ansi_char,
                                               1,
                                               unicode_char,
                                               1);
        if (r_code != 1)
        {
            crash("font_load(), не удалось преобразовать символ из cp1251 в unicode\nGetLastError() : %lu",
                  GetLastError());
        }
      
        if (unicode_char[1] != 0)
        {
            crash("font_load(), при конвертации ANSI символа [%c] получилась суррогатная пара UTF16-LE, а не единственный 16-ти битный символ",
                  ansi_char);
        }

        // Отрендерим символ в поверхность.
        SDL_Surface *h_surface = TTF_RenderGlyph_Blended(h_font, unicode_char[0], h_color);

        // Если для символа есть графическое представление.
        // Важно, для символа может быть графическое представление со всеми
        // прозрачными пикселями.
        if (h_surface != NULL)
        {
            /*printf("№%i char:%c unicode: %Iu height: %i width: %i\n",
                   i,
                   ansi_char,
                   (size_t)unicode_char[0],
                   h_surface->h,
                   h_surface->w);*/

            // Сформируем на основе поверхности текстуру.
            SDL_Texture *h_texture = SDL_CreateTextureFromSurface(renderer, h_surface);
            if (h_texture == NULL)
            {
                crash("font_load(), не удалось создать текстуру на основе поверхности с символом\nSDL_GetError() : %s",
                      SDL_GetError());
            }

            // Инициализируем глиф.
            glyphs[i].h = h_surface->h;
            glyphs[i].w = h_surface->w;
            glyphs[i].texture = h_texture;

            // Освобождаем вспомогательную поверхность.
            SDL_FreeSurface(h_surface);
        }
    }
  
  TTF_CloseFont(h_font);
}

// Загружает все необходимые текстуры.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void textures_load(void)
{
    // Защита от повторного вызова.
    static size_t again = 0;
    if (again++ != 0)
    {
        crash("textures_load(), произошла попытка повторной загрузки текстур");
    }

    // Текстура игрока.
    texture_load("textures/player.bmp", &texture_player);

    // Текстура врага.
    texture_load("textures/enemy.bmp", &texture_enemy);

    // Текстура стены.
    texture_load("textures/wall.bmp", &texture_wall);

    // Текстура дерева.
    texture_load("textures/tree.bmp", &texture_tree);

    // Текстура воды.
    texture_load("textures/water.bmp", &texture_water);

    // Текстура ремонта.
    texture_load("textures/repair.bmp", &texture_repair);

    // Текстура пули.
    texture_load("textures/bullet.bmp", &texture_bullet);

    // Текстура вспышки.
    texture_load("textures/flush.bmp", &texture_flush);

    // Текстура дыма.
    texture_load("textures/smoke.bmp", &texture_smoke);

    // Текстура захватывания.
    texture_load("textures/capture.bmp", &texture_capture);

}

// Загружает все необходимые звуки.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void sounds_load(void)
{
    // Защита от повторного вызова.
    static size_t again = 0;
    if (again++ != 0)
    {
        crash("sounds_load(), произошла попытка повторной загрузки звуков");
    }

    // Звук выстрела.
    sound_load("sounds/shot.wav", &sound_shot);

    // Звук попадания в стену.
    sound_load("sounds/hit_to_wall.wav", &sound_hit_to_wall);

    // Звук попадания во врага.
    sound_load("sounds/hit_to_enemy.wav", &sound_hit_to_enemy);

    // Звук попадания в игрока.
    sound_load("sounds/hit_to_player.wav", &sound_hit_to_player);

    // Звук взятия бонуса.
    sound_load("sounds/repair.wav", &sound_repair);
}

// Загружает карту.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void map_load(void)
{
    // Защита от повторного вызова.
    static size_t again = 0;
    if (again++ != 0)
    {
        crash("map_load(), произошла попытка повторной загрузки карты");
    }

    FILE *f = fopen("map/map.txt", "r");

    if (f == NULL)
    {
        crash("map_load(), не удалось загрузить карту\n%s",
              strerror(errno));
    }

    for (size_t y = 0; y < MAP_HEIGHT; ++y)
    {
        for (size_t x = 0; x < MAP_WIDTH; ++x)
        {
            int c = getc(f);

            // Карта должна быть окружена бортиком из стены.
            if ( (y == 0) || (y == MAP_HEIGHT - 1) ||
                 (x == 0) || (x == MAP_WIDTH - 1) )
            {
                if (c != '#')
                {
                    crash("map_load(), карта должна быть окружена стеной");
                }
            }

            switch (c)
            {
                case ('#'):
                {
                    map_template[x][y] = CS_WALL;
                    break;
                }
                case ('*'):
                {
                    map_template[x][y] = CS_TREE;
                    break;
                }
                case ('~'):
                {
                    map_template[x][y] = CS_WATER;
                    break;
                }
                case ('.'):
                {
                    map_template[x][y] = CS_EMPTY;
                    break;
                }
                case ('R'):
                {
                    map_template[x][y] = CS_REPAIR;
                    break;
                }
                default:
                {
                    crash("map_load(), файл карты поврежден");
                }
            }
        }
        // Проглатываем перевод строки.
        const int c = getc(f);
        if (c !=  '\n')
        {
            crash("map_load(), файл карты поврежден");
        }
    }

    // Углы карты прямо возле стены должны быть пусты - там респаунятся враги.
    if ( (map_template[1][1] != CS_EMPTY) ||
         (map_template[MAP_WIDTH - 2][1] != CS_EMPTY) ||
         (map_template[MAP_WIDTH - 2][MAP_HEIGHT - 2] != CS_EMPTY) ||
         (map_template[1][MAP_HEIGHT - 2] != CS_EMPTY) )
    {
        crash("map_load(), углы карты возле стены должны быть пустыми, там появляются враги");
    }

    // Центр карты (приблизительный) должен быть пуст - там появляется игрок.
    if ( map_template[MAP_WIDTH / 2][MAP_HEIGHT / 2] != CS_EMPTY)
    {
        crash("map_load(), центр карты должен быть пустым, там появляется игрок");
    }

    const int r_code = fclose(f);
    if (r_code == EOF)
    {
        crash("map_load(), не удалось закрыть файл с картой\n%s",
              strerror(errno));
    }
}
