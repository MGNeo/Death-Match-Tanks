#include <SDL_timer.h>
#include <time.h>
#include <stdlib.h>
#include "SDL.h"
#include "game.h"
#include "crash.h"
#include "resources_low.h"

#include "processing.h"
#include "draw.h"
#include "player.h"

#include "screen.h"

// Игровой цикл.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void game(void)
{
    srand(time(NULL));

    // Время отрисовки кадра (1.0 -> 1 секунда; 0.1 -> 0.1 секунда).
    float dt = 0.f;

    Uint64 t1 = SDL_GetPerformanceCounter(),
           t2;

    // Для доступа к актуальному состоянию клавиш.
    // Доступ должен осуществляться после цикла обработки событий.
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    // Задаем настройки смешивания.
    if (SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND) != 0)
    {
        crash("game(), не удалось установить режим смешивания");
    }

    // Флаг выхода.
    size_t quit = 0;
    // Событие.
    SDL_Event event;
    // Сцена.
    size_t scene = 0;
    while(!quit)
    {
        // Вычисляем dt.
        t2 = t1;
        t1 = SDL_GetPerformanceCounter();
        dt = (t1 - t2) / (float)SDL_GetPerformanceFrequency();

        // Независимо от сцены, если возникло событие завершения работы,
        // выходим из игрового цикла.
        while (SDL_PollEvent(&event) != 0)
        {
            if (event.type == SDL_QUIT)
            {
                quit = 1;
                break;
            }
        }

        // Нажатие escape завершает программу независимо от сцены.
        if (keys[SDL_SCANCODE_ESCAPE] == 1)
        {
            quit = 1;
        }

        // Для всех сцен цвет очистки - черный.
        if (SDL_SetRenderDrawColor(renderer, 0 ,0 ,0 ,0) != 0)
        {
            crash("game(), не удалось задать цвет очистки фона\nSDL_GetError() : %s",
                  SDL_GetError());
        }

        // Очищаем рендер перед обработкой всех сцен.
        if (SDL_RenderClear(renderer) != 0)
        {
            crash("game(), не удалось очистить рендер\nscene: %Iu\nSDL_GetError() : %s",
                  scene,
                  SDL_GetError());
        }

        // Каждая сцена обрабатывается по-особенному.
        switch(scene)
        {
            // Сцена с приветствием.
            case(0):
            {
                // Рисуем приветствие.
                draw_hello();

                // Нажатие Enter запускает сцену с боем.
                if (keys[SDL_SCANCODE_RETURN] == 1)
                {
                    // Инициализируем бой.
                    reset_player();
                    reset_enemies();
                    reset_bullets();
                    reset_map();
                    reset_effects();
                    reset_score();

                    for (size_t i = 0; i < 8; ++i)
                    add_enemy();

                    // Включаем сцену боя.
                    scene = 1;
                }
                break;
            }

            // Сцена боя.
            case(1):
            {
                draw_map(CS_WALL);
                draw_map(CS_WATER);
                draw_map(CS_REPAIR);

                processing_player(keys, dt);
                processing_enemies(dt);
                processing_bullets(dt);
                processing_effects(dt);

                draw_player();
                draw_enemies();
                draw_bullets();
                draw_effects();

                draw_indicators();
                draw_map(CS_TREE);
                draw_score();

                // Если игрока убили.
                if (player_is_dead() == 1)
                {
                    scene = 2;
                    reset_curtain();
                }
                break;
            }

            // Сцена смерти игрока (время остановилось, все угасает).
            case(2):
            {
                draw_map(CS_WALL);
                draw_map(CS_WATER);
                draw_map(CS_REPAIR);

                draw_enemies();
                draw_bullets();
                draw_effects();

                draw_indicators();
                draw_map(CS_TREE);
                draw_score();

                // Обрабатываем занавес.
                processing_curtain(dt);
                // Рисуем занавес.
                draw_curtain();

                // Если занавес опустился, переходим к сцене со статистикой.
                if (curtain_is_max())
                {
                    scene = 3;
                }

                break;
            }
            // Сцена статистики.
            case (3):
            {
                draw_statistics();

                // Нажатие Enter запускает сцену с боем.
                if (keys[SDL_SCANCODE_RETURN] == 1)
                {
                    // Инициализируем бой.
                    reset_player();
                    reset_enemies();
                    reset_bullets();
                    reset_map();
                    reset_effects();
                    reset_score();

                    for (size_t i = 0; i < 8; ++i)
                    add_enemy();

                    // Включаем сцену боя.
                    scene = 1;
                }

                break;
            }
            default:
            {
                crash("game(), задана некорректная сцена\nscene : %Iu",
                      scene);
            }
        }

        // Представляем рендер после обработки всех сцен.
        SDL_RenderPresent(renderer);
    }
}
