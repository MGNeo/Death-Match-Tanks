#include <SDL_timer.h>
#include <time.h>
#include <stdlib.h>
#include <SDL.h>
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
        crash("game(), не удалось установить режим смешивания\nSDLGetError() : %s",
              SDL_GetError());
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
                hello_draw();

                // Нажатие Enter запускает сцену с боем.
                if (keys[SDL_SCANCODE_RETURN] == 1)
                {
                    // Инициализируем бой.
                    player_reset();
                    enemies_reset();
                    bullets_reset();
                    map_reset();
                    effects_reset();
                    score_reset();

                    for (size_t i = 0; i < 8; ++i)
                    enemy_add();

                    // Включаем сцену боя.
                    scene = 1;
                }
                break;
            }

            // Сцена боя.
            case(1):
            {
                map_draw(CS_WALL);
                map_draw(CS_WATER);
                map_draw(CS_REPAIR);

                player_processing(keys, dt);
                enemies_processing(dt);
                bullets_processing(dt);
                effects_processing(dt);

                player_draw();
                enemies_draw();
                bullets_draw();
                effects_draw();

                indicators_draw();
                map_draw(CS_TREE);
                score_draw();

                // Если игрока убили.
                if (player_is_dead() == 1)
                {
                    scene = 2;
                    curtain_reset();
                }
                break;
            }

            // Сцена смерти игрока (время остановилось, все угасает).
            case(2):
            {
                map_draw(CS_WALL);
                map_draw(CS_WATER);
                map_draw(CS_REPAIR);

                enemies_draw();
                bullets_draw();
                effects_draw();

                indicators_draw();
                map_draw(CS_TREE);
                score_draw();

                // Обрабатываем занавес.
                curtain_processing(dt);
                // Рисуем занавес.
                curtain_draw();

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
                statistics_draw();

                // Нажатие Enter запускает сцену с боем.
                if (keys[SDL_SCANCODE_RETURN] == 1)
                {
                    // Инициализируем бой.
                    player_reset();
                    enemies_reset();
                    bullets_reset();
                    map_reset();
                    effects_reset();
                    score_reset();

                    for (size_t i = 0; i < 8; ++i)
                    enemy_add();

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
