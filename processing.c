#include <math.h>
#include <stdlib.h>
#include <memory.h>

#include "processing.h"
#include "player.h"
#include "enemies.h"
#include "bullets.h"
#include "map.h"
#include "sprite.h"
#include "health.h"
#include "speed.h"
#include "damage.h"
#include "cooldown.h"
#include "resources_low.h"
#include "crash.h"
#include "repair.h"
#include "owner.h"
#include "effects.h"
#include "score.h"
#include "curtain.h"

// Добавляет любой эффект.
static void add_effect(const float _x,
                       const float _y,
                       const float _angle,
                       const float _alpha,
                       const float _d_angle,
                       const float _d_alpha,
                       const SDL_Texture *const _texture);

// Добавляет эффект дыма.
static void add_effect_smoke(const float _x,
                             const float _y);
// Добавляет эффект вспышки.
static void add_effect_flush(const float _x,
                             const float _y);
// Добавляет эффект захвата.
static void add_effect_capture(const float _x,
                               const float _y);

// Добавляет эффекты взрыва танка.
static void add_effects_tank_explode(const tank *const _tank);

// Добавляет пулю, размещая ее на кончике дула танка.
static void add_bullet(const tank *const _tank,// x, y, damage, direction и пр.
                       const owner _owner);

// Обработка движения игрока и врага - это обработка движения танка.
static void move_tank(tank *const _tank, const float _dt);
// Обработка перезарядки игрока и врага - это обработка перезарядки танка.
static void recharge_tank(tank *const _tank, const float _dt);
// Обработка сбора ремонтных наборов игроком и врагом - это обработка сбора ремонтных наборов танком.
static void repair_tank(tank *const _tank);

// Обработка управления игроком.
static void control_player(const Uint8 *const _keys);
// Обработка стрельбы игроком.
static void shoot_player(const Uint8 *const _keys);

// Обработка управления врагом.
static void control_enemy(tank *const _enemy);
// Обработка стрельбы врагом.
static void shoot_enemy(tank *const _enemy);

// Обработка попадания пули.
static void hit_bullet(bullet *const _bullet);
// Обработка движения пули.
static void move_bullet(bullet *const _bullet, const float _dt);
// Воспроизведение звука.
static void sound_play(Mix_Chunk *const _sound);

// Обрабатывает игрока - его движение, перезарядку, стрельбу, сбор ремонтных наборов.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void processing_player(const Uint8 *const _keys, const float _dt)
{
    if (_keys == NULL)
    {
        crash("processing_player(), _keys == NULL");
    }

    // Обрабатываем управление игроком.
    control_player(_keys);

    // Обрабатываем передвижение игрока.
    move_tank(&player, _dt);

    // Обрабатываем перезарядку игрока.
    recharge_tank(&player, _dt);

    // Обрабатываем стрельбу игрока.
    shoot_player(_keys);

    // Обрабатываем сбор ремонтных наборов игроком.
    repair_tank(&player);
}

// Обрабатывает врагов - их движение, перезарядку, стрельбу, сбор ремонтных наборов.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void processing_enemies(const float _dt)
{
    if (enemies_count > 0)
    {
        for (size_t e = 0; e < MAX_ENEMIES; ++e)
        {
            if (enemies[e].active == 1)
            {
                // Обрабатываем управление врагом.
                control_enemy(&enemies[e]);

                // Обрабатываем передвижение врага.
                move_tank(&enemies[e], _dt);

                // Обрабатываем перезарядку врага.
                recharge_tank(&enemies[e], _dt);

                // Обрабатываем стрельбу врага.
                shoot_enemy(&enemies[e]);

                // Обрабатываем сбор ремонтных наборов врагом.
                repair_tank(&enemies[e]);
            }
        }
    }
}

// Обрабатывает пули (движение и попадания).
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void processing_bullets(const float _dt)
{
    if (bullets_count > 0)
    {
        for (size_t b = 0; b < MAX_BULLETS; ++b)
        {
            if (bullets[b].active == 1)
            {
                // Обрабатываем попадание пули.
                hit_bullet(&bullets[b]);

                // Обрабатываем движение пули.
                move_bullet(&bullets[b], _dt);
            }
        }
    }
}

// Обрабатывает эффекты.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void processing_effects(const float _dt)
{
    if (effects_count > 0)
    {
        for (size_t e = 0; e < MAX_EFFECTS; ++e)
        {
            if (effects[e].active ==
                1)
            {
                effects[e].angle += _dt * effects[e].d_angle;
                effects[e].alpha -= _dt * effects[e].d_alpha;

                // Эффект становится неактивен тогда, когда он становится полностью прозрачен.
                if (effects[e].alpha <= 0)
                {
                    effects[e].active = 0;
                    --effects_count;
                }
            }
        }
    }
}

// Обрабатывает движение танка к его цели.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void move_tank(tank *const _tank, const float _dt)
{
    if (_tank == NULL)
    {
        crash("move_tank(), _tank == NULL");
    }

    // Разность между x и tx танка.
    const float dif_x_1 = _tank->x - _tank->tx;
    // Разность между y и ty танка.
    const float dif_y_1 = _tank->y - _tank->ty;

    // Расстояние до цели по оси x.
    const float dis_x = fabs(dif_x_1);
    // Расстояние до цели по оси y.
    const float dis_y = fabs(dif_y_1);

    // Если танк по обеим осям находится на цели, то он не едет.
    if ( (dis_x == 0.f) &&
         (dis_y == 0.f) )
    {
        return;
    }

    // Запоминаем положение танка относительно его цели, чтобы проконтролировать
    // возможный "переезд".

    int dir_x_1 = 0;
    // Танк левее цели.
    if (dif_x_1 < 0.f)
    {
        dir_x_1 = -1;
    } else {
        // Танк правее цели.
        if (dif_x_1 > 0.f)
        {
            dir_x_1 = 1;
        }
    }

    int dir_y_1 = 0;
    // Танк выше цели.
    if (dif_y_1 < 0.f)
    {
        dir_y_1 = -1;
    } else {
        // Танк ниже цели.
        if (dif_y_1 > 0.f)
        {
            dir_y_1 = 1;
        }
    }

    // Едем (лишь по одной оси единовременно).

    // Движение по x.
    if (dif_x_1 != 0)
    {
        _tank->x -= _tank->speed * dir_x_1 * _dt;
    } else {
        // Движение по y.
        if (dif_y_1 != 0)
        {
            _tank->y -= _tank->speed * dir_y_1 * _dt;
        }
    }

    // Контроль ситуации, в которой танк "переехал" цель.
    // Это возможно при лаге системы, при низком fps, при высокой скорости и пр.

    // Разность между новым положением и целью по x.
    const float dif_x_2 = _tank->x - _tank->tx;
    // Разность между новым положением и целью по y.
    const float dif_y_2 = _tank->y - _tank->ty;


    // Определяем новое положение танка относительно его цели по x.
    int dir_x_2 = 0;
    if (dif_x_2 < 0.f)
    {
        dir_x_2  = -1;
    } else {
        if (dif_x_2 > 0.f)
        {
            dir_x_2 = 1;
        }
    }

    // Определяем новое положение танка относительно его цели по y.
    int dir_y_2 = 0;
    if (dif_y_2 < 0.f)
    {
        dir_y_2  = -1;
    } else {
        if (dif_y_2 > 0.f)
        {
            dir_y_2 = 1;
        }
    }

    // Если танк переехал цель, он оказывается на цели.
    if (dir_x_1 != dir_x_2)
    {
        _tank->x = _tank->tx;
    }
    if (dir_y_1 != dir_y_2)
    {
        _tank->y = _tank->ty;
    }
}

// Обрабатывает перезарядку танка.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void recharge_tank(tank *const _tank, const float _dt)
{
    if (_tank == NULL)
    {
        crash("recharge_tank(), _tank == NULL");
    }

    // Перезарядка оружия.
    if (_tank->c > 0.f)
    {
        _tank->c -= _dt;
    } else {
        _tank->c = 0.f;
    }

}

// Контролирует игрока (выбор цели для движения).
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void control_player(const Uint8 *const _keys)
{
    if (_keys == NULL)
    {
        crash("control_player(), не задан массив состояний кнопок");
    }

    // Разность x и tx игрока.
    const float dif_x = player.x - player.tx;
    // Разность y и ty игрока.
    const float dif_y = player.y - player.ty;

    // Расстояние до цели по x.
    const float dis_x = fabs(dif_x);
    // Расстояние до цели по y.
    const float dis_y = fabs(dif_y);

    // Если игрок по обеим осям на своей цели, обрабатываем
    // управление и взятие бонуса/ремонта.
    if ( (dis_x == 0.f) &&
         (dis_y == 0.f) )
    {
        // Определяем клетку, в которой находится игрок.
        const size_t cx = player.x / SPRITE_SIZE + 0.5f;
        const size_t cy = player.y / SPRITE_SIZE + 0.5f;

        // Контроль значения cx.
        // ...

        if (_keys[SDL_SCANCODE_A] == 1)
        {
            // Если нажали кнопку и игрок стоял, в любом случае поворачиваем.
            player.d = D_LEFT;
            // А если еще и можно ехать в заданную точку, то едем.
            if ( (map[cx - 1][cy] != CS_WALL) &&
                 (map[cx - 1][cy] != CS_WATER) )
            {
                player.tx -= SPRITE_SIZE;
                goto END;
            }
        }

        if (_keys[SDL_SCANCODE_D] == 1)
        {
            // Если нажали кнопку и игрок стоял, в любом случае поворачиваем.
            player.d = D_RIGHT;
            // А если еще и можно ехать в заданную точку, то едем.
            if ( (map[cx + 1][cy] != CS_WALL) &&
                 (map[cx + 1][cy] != CS_WATER) )
            {
                player.tx += SPRITE_SIZE;
                goto END;
            }
        }
        if (_keys[SDL_SCANCODE_W] == 1)
        {
            // Если нажали кнопку и игрок стоял, в любом случае поворачиваем.
            player.d = D_TOP;
            // А если еще и можно ехать в заданную точку, то устанавливаем новую цель.
            if ( (map[cx][cy - 1] != CS_WALL) &&
                 (map[cx][cy - 1] != CS_WATER) )
            {
                player.ty -= SPRITE_SIZE;
                goto END;
            }
        }
        if (_keys[SDL_SCANCODE_S] == 1)
        {
            // Если нажали кнопку и игрок стоял, в любом случае поворачиваем.
            player.d = D_BOTTOM;
            // А если еще и можно ехать в заданную точку, то устанавливаем новую цель.
            if ( (map[cx][cy + 1] != CS_WALL) &&
                 (map[cx][cy + 1] != CS_WATER) )
            {
                player.ty += SPRITE_SIZE;
                goto END;
            }
        }
    }

    END:;

}

// Контролирует стрельбу игрока.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void shoot_player(const Uint8 *const _keys)
{
    if (_keys == NULL)
    {
        crash("shoot_player(), _keys == NULL");
    }

    // Если орудие игрока перезарядилось.
    if (player.c <= 0.f)
    {
        // И нажат пробел.
        if (_keys[SDL_SCANCODE_SPACE] == 1)
        {
            // Забираем у игрока 1 очко.
            if (score > 0)
            {
                score -= 1;
            }

            // Добавляем пулю (не учитываем сложение скоростей).
            add_bullet(&player,
                       O_PLAYER);

            // Начинаем перезарядку.
            player.c = SHOT_CD;
        }
    }
}

// Контролирует врага (выбор цели для движения).
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void control_enemy(tank *const _enemy)
{
    if (_enemy == NULL)
    {
        crash("control_enemy(), _enemy == NULL");
    }

    // Определяем, в какой клетке находится враг.
    const size_t cx = _enemy->x / SPRITE_SIZE + 0.5f;
    const size_t cy = _enemy->y / SPRITE_SIZE + 0.5f;

    // Определяем расстояние врага до цели.
    const float dif_x = _enemy->x - _enemy->tx;
    const float dif_y = _enemy->y - _enemy->ty;

    // Если враг достиг целевой клетки, выбираем новую клетку.
    if ( (dif_x == 0.f) &&
         (dif_y == 0.f) )
    {
        // Определяем, в какой клетке находилась его прошлая позиция.
        const size_t ox = _enemy->ox / SPRITE_SIZE + 0.5f;
        const size_t oy = _enemy->oy / SPRITE_SIZE + 0.5f;

        // Определяем возможные направления движения,
        // исключающие клетки с водой, стеной и прошлой позицией.
        direction ds[4];
        size_t dcount = 0;

        // Верх.
        cell_state cell;
        cell = map[cx][cy - 1];
        if ( (cell != CS_WALL) &&
             (cell != CS_WATER) &&
             ((cy - 1) != oy) )
        {
             ds[dcount] = D_TOP;
             ++dcount;
        }

        // Право.
        cell = map[cx + 1][cy];
        if ( (cell != CS_WALL) &&
             (cell != CS_WATER) &&
             ((cx + 1) != ox) )
        {
             ds[dcount] = D_RIGHT;
             ++dcount;
        }

        // Низ.
        cell = map[cx][cy + 1];
        if ( (cell != CS_WALL) &&
             (cell != CS_WATER) &&
             ((cy + 1) != oy) )
        {
             ds[dcount] = D_BOTTOM;
             ++dcount;
        }

        // Лево.
        cell = map[cx - 1][cy];
        if ( (cell != CS_WALL) &&
             (cell != CS_WATER) &&
             ((cx - 1) != ox) )
        {
             ds[dcount] = D_LEFT;
             ++dcount;
        }

        // Текущая позиция запоминается, как старая.
        _enemy->ox = _enemy->x;
        _enemy->oy = _enemy->y;

        // Если нет иных направлений, кроме того, откуда приехал враг, то только в этом случае
        // враг поворачивает назад.
        if (dcount == 0)
        {
            _enemy->tx = _enemy->ox;
            _enemy->ty = _enemy->oy;

            return;
        } else {
            // Иначе выбирается случайное направление из доступных.
            const size_t r = rand() % dcount;
            // Поворачиваем врага лицом в направлении движения.
            _enemy->d = ds[r];
            // Устанавливаем координаты цели.
            switch (_enemy->d)
            {
                case(D_TOP):
                {
                    _enemy->tx = cx * SPRITE_SIZE;
                    _enemy->ty = (cy - 1) * SPRITE_SIZE;
                    break;
                }
                case(D_RIGHT):
                {
                    _enemy->tx = (cx + 1) * SPRITE_SIZE;
                    _enemy->ty = cy * SPRITE_SIZE;
                    break;
                }
                case(D_BOTTOM):
                {
                    _enemy->tx = cx * SPRITE_SIZE;
                    _enemy->ty = (cy + 1) * SPRITE_SIZE;
                    break;
                }
                case(D_LEFT):
                {
                    _enemy->tx = (cx - 1) * SPRITE_SIZE;
                    _enemy->ty = cy * SPRITE_SIZE;
                    break;
                }
                default:
                {
                    crash("control_enemy(), неизвестное направление движения при выборе целевой клетки\n_enemy->d: %i",
                          _enemy->d);
                }
            }
        }
    }
}


// Контролирует стрельбу врага.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void shoot_enemy(tank *const _enemy)
{
    if (_enemy == NULL)
    {
        crash("shoot_enemy(), _enemy == NULL");
    }

    // Если орудие врага перезарядилось.
    if (_enemy->c <= 0.f)
    {
        // Определяем, в какой клетке находиться враг.
        const size_t cx = _enemy->x / SPRITE_SIZE + 0.5f;
        const size_t cy = _enemy->y / SPRITE_SIZE + 0.5f;

        // Определяем, в какой клетке находится игрок.
        const size_t px = player.x / SPRITE_SIZE + 0.5f;
        const size_t py = player.y / SPRITE_SIZE + 0.5f;

        size_t fire = 0;

        switch (_enemy->d)
        {
            // Если враг смотрит вверх.
            case (D_TOP):
            {
                // Если игрок находится на одной линии с врагом по x.
                if (px == cx)
                {
                    // И игрок выше врага.
                    if (py <= cy)
                    {
                        // Определяем, есть ли между врагом и игроком стены.
                        size_t walls = 0;
                        for (size_t w = py; w <= cy; ++w)
                        {
                            if (map[cx][w] == CS_WALL)
                            {
                                ++walls;
                            }
                        }
                        // Если стен нет, враг стреляет.
                        if (walls == 0)
                        {
                            fire = 1;
                        }
                    }
                }
                break;
            }
            // Если враг смотрит вправо.
            case (D_RIGHT):
            {
                // Если игрок находится на одной линии с врагом по y.
                if (py == cy)
                {
                    // И игрок правее врага.
                    if (px >= cx)
                    {
                        // Определяем, есть ли между врагом и игроком стены.
                        size_t walls = 0;
                        for (size_t w = cx; w <= px; ++w)
                        {
                            if (map[w][cy] == CS_WALL)
                            {
                                ++walls;
                            }
                        }
                        // Если стен нет, враг стреляет.
                        if (walls == 0)
                        {
                            fire = 1;
                        }
                    }
                }
                break;
            }
            // Если враг смотрит вниз.
            case (D_BOTTOM):
            {
                // Если игрок находится на одной линии с врагом по x.
                if (px == cx)
                {
                    // И игрок ниже врага.
                    if (py >= cy)
                    {
                        // Определяем, есть ли между врагом и игроком стены.
                        size_t walls = 0;
                        for (size_t w = cy; w <= py; ++w)
                        {
                            if (map[cx][w] == CS_WALL)
                            {
                                ++walls;
                            }
                        }
                        // Если стен нет, враг стреляет.
                        if (walls == 0)
                        {
                            fire = 1;
                        }
                    }
                }
                break;
            }
            // Если враг смотрит влево.
            case (D_LEFT):
            {
                // Если игрок находится на одной линии с врагом по y.
                if (py == cy)
                {
                    // И игрок левее врага.
                    if (px <= cx)
                    {
                        // Определяем, есть ли между врагом и игроком стены.
                        size_t walls = 0;
                        for (size_t w = px; w <= cx; ++w)
                        {
                            if (map[w][cy] == CS_WALL)
                            {
                                ++walls;
                            }
                        }
                        // Если стен нет, враг стреляет.
                        if (walls == 0)
                        {
                            fire = 1;
                        }
                    }
                }
                break;
            }
            default:
            {
                crash("shoot_enemy(), неизвестное направление при прицеливании в игрока\n_enemy->d: %i",
                      _enemy->d);
            }
        }

        // Если враг выстрелил, обрабатываем.
        if (fire == 1)
        {
            // Добавляем пулю (не учитываем сложение скоростей).
            add_bullet(_enemy,
                       O_ENEMY);

            _enemy->c = SHOT_CD;
        }
    }
}

// Сбрасывает состояние игрока в изначальное.
void reset_player(void)
{
    player.hp = MAX_HEALTH;
    player.speed = TANK_SPEED;
    player.d = D_TOP;
    player.c = 0.f;
    player.x = (MAP_WIDTH / 2 ) * SPRITE_SIZE;
    player.y = (MAP_HEIGHT / 2) * SPRITE_SIZE;
    player.tx = player.x;
    player.ty = player.y;
}

// Сбрасывает состояние врагов.
void reset_enemies(void)
{
    for (size_t e = 0; e < MAX_ENEMIES; ++e)
    {
        enemies[e].active = 0;
    }
    enemies_count = 0;
}

// Сбрасывает состояние пуль.
void reset_bullets(void)
{
    for (size_t b = 0; b < MAX_BULLETS; ++b)
    {
        bullets[b].active = 0;
    }
    bullets_count = 0;
}

// Сбрасывает состояние карты.
void reset_map(void)
{
    memcpy(map, map_template, sizeof(cell_state) * MAP_WIDTH * MAP_HEIGHT);
}

//Сбрасывает счет.
void reset_score(void)
{
    score = 0;
}

// Обрабатывает попадание пули в игрока, врагов и стены.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void hit_bullet(bullet *const _bullet)
{
    if (_bullet == NULL)
    {
        crash("hit_bullet(), _bullet == NULL");
    }

    switch (_bullet->owner)
    {
        // Пуля принадлежит игроку.
        case (O_PLAYER):
        {
            if (enemies_count > 0)
            {
                for (size_t e = 0; e < MAX_ENEMIES; ++e)
                {
                    // Если враг активен.
                    if (enemies[e].active == 1)
                    {
                        const float dis_x = fabs(_bullet->x - enemies[e].x);
                        const float dis_y = fabs(_bullet->y - enemies[e].y);

                        // Пуля попала во врага.
                        if ( (dis_x <= SPRITE_SIZE / 3) &&
                             (dis_y <= SPRITE_SIZE / 3) )
                        {
                            // При попадании начисляем 10 очков.
                            score += 10;

                            // Добавляем эффект дыма.
                            add_effect_smoke(_bullet->x,
                                             _bullet->y);
                            // Добавляем эффект вспышки.
                            add_effect_flush(_bullet->x,
                                             _bullet->y);

                            // Деактивируем пулю.
                            _bullet->active = 0;
                            // Уменьшаем счетчик пуль.
                            --bullets_count;

                            // Наносим врагу урон.
                            enemies[e].hp -= DAMAGE;
                            // Если врага убило.
                            if (enemies[e].hp <= 0)
                            {
                                // При убийстве начисляем 100 очков.
                                score += 100;

                                // Добавляем взрыв.
                                add_effects_tank_explode(&enemies[e]);

                                // Вырубаем врага.
                                enemies[e].active = 0;
                                // Уменьшаем счетчик врагов.
                                --enemies_count;
                                // Создаем двух новых.
                                add_enemy();
                                add_enemy();
                            }

                            // Воспроизводим звук попадания во врага.
                            sound_play(sound_hit_to_enemy);

                            return;
                        }
                    }
                }
            }
            break;
        }
        // Пуля принадлежит врагу.
        case (O_ENEMY):
        {
            const float dis_x = fabs(_bullet->x - player.x);
            const float dis_y = fabs(_bullet->y - player.y);

            // Пуля попала в игрока.
            if ( (dis_x <= SPRITE_SIZE / 3) &&
                 (dis_y <= SPRITE_SIZE / 3) )
            {
                // Добавляем эффект дыма.
                add_effect_smoke(_bullet->x,
                                 _bullet->y);
                // Добавляем эффект вспышки.
                add_effect_flush(_bullet->x,
                                 _bullet->y);
                // Деактивируем пулю.
                _bullet->active = 0;
                // Уменьшаем счетчик пуль.
                --bullets_count;

                // Наносим игроку урон.
                player.hp -= DAMAGE;
                // Если игрока убило.
                if (player.hp <= 0)
                {
                    player.hp = 0;

                    // Добавляем взрыв.
                    add_effects_tank_explode(&player);
                }

                // Воспроизводим звук попадания в игрока.
                sound_play(sound_hit_to_player);

                return;
            }
            break;
        }
        default:
        {
            crash("hit_bullet(), владелец пули неизвестен\n_bullet->owner: %i",
                  _bullet->owner);
        }
    }

    // Если пуля игрока не попала во врага, а пуля врага не попала в игрока, то
    // проверяем, попала ли она в стену.

    const size_t cx = _bullet->x / SPRITE_SIZE + 0.5f;
    const size_t cy = _bullet->y / SPRITE_SIZE + 0.5f;

    // Пуля попала в стену.
    if (map[cx][cy] == CS_WALL)
    {
        // Добавляем эффект дыма.
        add_effect_smoke(_bullet->x,
                         _bullet->y);
        // Добавляем эффект вспышки.
        add_effect_flush(_bullet->x,
                         _bullet->y);
        // Деактивируем пулю.
        _bullet->active = 0;
        // Уменьшаем счетчик пуль.
        --bullets_count;
        // Воспроизводим звук попадания в стену.
        sound_play(sound_hit_to_wall);
    }
}

// Обрабатывает перемещение пули.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void move_bullet(bullet *const _bullet, const float _dt)
{
    if (_bullet == NULL)
    {
        crash("move_bullet(), _bullet == NULL");
    }

    const float value = BULLET_SPEED * _dt;
    switch (_bullet->d)
    {
        case (D_TOP):
        {
            _bullet->y -= value;
            break;
        }
        case (D_RIGHT):
        {
           _bullet->x += value;
            break;
        }
        case (D_BOTTOM):
        {
            _bullet->y += value;
            break;
        }
        case (D_LEFT):
        {
            _bullet->x -= value;
            break;
        }
        default:
        {
            crash("move_bullet(), неизвестное направление движения пули\n_bullet->d: %i",
                  _bullet->d);
        }
    }
}

// Добавляет (активирует) нового врага.
// В случае ошибки показывает причину отказа и крашит программу.
void add_enemy(void)
{
    if (enemies_count == MAX_ENEMIES)
    {
        return;
    }

    for (size_t e = 0; e < MAX_ENEMIES; ++e)
    {
        if (enemies[e].active == 0)
        {
            enemies[e].active = 1;
            enemies[e].hp = MAX_HEALTH;
            // Для решения проблемы синхронного дерганья врагов.
            enemies[e].speed = TANK_SPEED - rand() % (TANK_SPEED / 10);


            enemies[e].d = D_BOTTOM;
            enemies[e].c = 0;

            const int r = enemies_count % 4;
            switch(r)
            {
                // Левый верхний угол.
                case (0):
                {
                    enemies[e].x = SPRITE_SIZE * 1;
                    enemies[e].y = SPRITE_SIZE * 1;
                    break;
                }
                // Правый верхний угол.
                case (1):
                {
                    enemies[e].x = SPRITE_SIZE * (MAP_WIDTH - 2);
                    enemies[e].y = SPRITE_SIZE * (1);
                    break;
                }
                // Левый нижний угол.
                case (2):
                {
                    enemies[e].x = SPRITE_SIZE * 1;
                    enemies[e].y = SPRITE_SIZE * (MAP_HEIGHT - 2);
                    break;
                }
                // Правый нижний угол.
                case (3):
                {
                    enemies[e].x = SPRITE_SIZE * (MAP_WIDTH - 2);
                    enemies[e].y = SPRITE_SIZE * (MAP_HEIGHT - 2);
                    break;
                }
            }

            enemies[e].tx = enemies[e].x;
            enemies[e].ty = enemies[e].y;
            enemies[e].ox = enemies[e].x;
            enemies[e].oy = enemies[e].y;

            ++enemies_count;

            break;
        }
    }
}

// Добавляет (активирует) новую пулю.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void add_bullet(const tank *const _tank,
                const owner _owner)
{
    if (bullets_count == MAX_BULLETS)
    {
        return;
    }

    for (size_t b = 0; b < MAX_BULLETS; ++b)
    {
        // Нашли неактивную пулю.
        if (bullets[b].active == 0)
        {

            // Определяем примерные координаты выхода из ствола.
            float x = _tank->x;
            float y = _tank->y;

            const float bias = SPRITE_SIZE / 2;

            switch(_tank->d)
            {
                case (D_TOP):
                {
                    y -= bias;
                    break;
                }
                case (D_RIGHT):
                {
                    x += bias;
                    break;
                }
                case (D_BOTTOM):
                {
                    y += bias;
                    break;
                }
                case (D_LEFT):
                {
                    x -= bias;
                    break;
                }
                default:
                {
                    crash("add_bullet(), неизвестное направление выстрела");
                }
            }

            // Инициализируем пулю.
            bullets[b].x = x;
            bullets[b].y = y;
            bullets[b].d = _tank->d;
            bullets[b].owner = _owner;
            bullets[b].active = 1;

            ++bullets_count;

            // Добавляем эффект дыма из точки вылета пули.
            add_effect_smoke(x,
                             y);

            // Воспроизводим звук выстрела.
            sound_play(sound_shot);

            break;
        }
    }
}

// Контролирует сбор ремонтных наборов танком.
// Набор собирается только в том случае, если танк поврежден.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
static void repair_tank(tank *const _tank)
{
    if (_tank == NULL)
    {
        crash("repair_tank(), _tank == NULL");
    }

    if (_tank->hp == MAX_HEALTH)
    {
        return;
    }

    // Определяем клетку, в которой находится танк.
    const size_t cx = _tank->x / SPRITE_SIZE + 0.5f;
    const size_t cy = _tank->y / SPRITE_SIZE + 0.5f;

    if (map[cx][cy] == CS_REPAIR)
    {
        map[cx][cy] = CS_EMPTY;
        _tank->hp += REPAIR_COUNT;

        if (_tank->hp > MAX_HEALTH)
        {
            _tank->hp = MAX_HEALTH;
        }
        // Добавляем эффект захвата.
        add_effect_capture(cx * SPRITE_SIZE,
                           cy * SPRITE_SIZE);

        // Воспроизводим звук захвата.
        sound_play(sound_repair);
    }
}

// Сбрасывает все эффекты.
void reset_effects(void)
{
    for (size_t e = 0; e < MAX_EFFECTS; ++e)
    {
        effects[e].active = 0;
        effects_count = 0;
    }
}

// Добавляет эффект.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
static void add_effect(const float _x,
                       const float _y,
                       const float _angle,
                       const float _alpha,// [0; 255]
                       const float _d_angle,
                       const float _d_alpha,// >= 0
                       const SDL_Texture *const _texture)
{
    if (_alpha < 0)
    {
        crash("add_effect(), _alpha < 0.f\n_alpha: %f",
              _alpha);
    }
    if (_alpha > 255)
    {
        crash("add_effect(), _alpha > 255.f\n_alpha: %f",
              _alpha);
    }
    if (_d_alpha < 0.f)
    {
        crash("add_effect(), _d_alpha < 0.f\n_d_alpha: %f",
              _d_alpha);
    }
    if (_texture == NULL)
    {
        crash("add_effect(), _texture == NULL");
    }

    if (effects_count == MAX_EFFECTS)
    {
        return;
    }

    for (size_t e = 0; e < MAX_EFFECTS; ++e)
    {
        if (effects[e].active == 0)
        {
            effects[e].active = 1;
            ++effects_count;

            effects[e].x = _x;
            effects[e].y = _y;

            effects[e].angle = _angle;
            effects[e].alpha = _alpha;

            effects[e].d_angle = _d_angle;
            effects[e].d_alpha = _d_alpha;

            effects[e].texture = (SDL_Texture*)_texture;

            return;
        }
    }
}

// Добавляет эффект дыма.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
static void add_effect_smoke(const float _x,
                             const float _y)
{
    add_effect(_x,
               _y,
               rand() % 360,
               255,
               pow(-1, rand() % 2) * 20,
               140,
               texture_smoke);
}

// Добавляет эффект вспышки.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void add_effect_flush(const float _x,
                      const float _y)
{
    add_effect(_x,
               _y,
               rand() % 360,
               255,
               0,
               2 * 255,
               texture_flush);
}

// Добавляет эффект захватывания.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
static void add_effect_capture(const float _x,
                               const float _y)
{
    add_effect(_x,
               _y,
               rand() % 360,
               255,
               pow(-1, rand() % 2) * 122,
               100,
               texture_capture);
}

// Если игрок убит, возвращат 1, иначе 0.
int player_is_dead(void)
{
    if (player.hp <= 0)
    {
        return 1;
    }
    return 0;
}

// Добавляет эффекты взрыва танка.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
void add_effects_tank_explode(const tank *const _tank)
{
    if (_tank == NULL)
    {
        crash("add_effects_tank_explode(), _tank == NULL");
    }

    const float bias = SPRITE_SIZE / 4;

    // Добавляем эффекты дыма
    add_effect_smoke(_tank->x,
                     _tank->y);
    add_effect_smoke(_tank->x + bias,
                     _tank->y - bias);
    add_effect_smoke(_tank->x - bias,
                     _tank->y - bias);
    add_effect_smoke(_tank->x + bias,
                     _tank->y + bias);
    add_effect_smoke(_tank->x - bias,
                     _tank->y + bias);

    // Добавляем эффекты вспышки.
    add_effect_flush(_tank->x,
                     _tank->y);
    add_effect_flush(_tank->x + bias,
                     _tank->y - bias);
    add_effect_flush(_tank->x - bias,
                     _tank->y - bias);
    add_effect_flush(_tank->x + bias,
                     _tank->y + bias);
    add_effect_flush(_tank->x - bias,
                     _tank->y + bias);
}

// Сбрасывает занавес.
void reset_curtain(void)
{
    curtain = 0.f;
}

// Обрабатывает занавес.
void processing_curtain(const float _dt)
{
    curtain += _dt * CURTAIN_SPEED;
    if (curtain >= 255.f)
    {
        curtain = 255.f;
    }
}

// Если занавес полностью опустился, возвращает 1, иначе 0.
int curtain_is_max(void)
{
    if (curtain >= 255.f)
    {
        return 1;
    }

    return 0;
}

// Воспроизводит звук.
// В случае ошибки показывает информацию о причине сбоя и крашит программу.
static void sound_play(Mix_Chunk *const _sound)
{
    if (_sound == NULL)
    {
        crash("sound_play(), _sound == NULL");
    }

    // Возвращает -1 непонятно в какой ситуации.
    /*if (Mix_PlayChannel(-1, _sound, 0) == -1)
    {
        crash("Не удалось воспроизвести звук.\nMix_GetError() : %s",
              Mix_GetError());
    }*/
    Mix_PlayChannel(-1, _sound, 0);
}
