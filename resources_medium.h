/*
 * Среднеуровневый модуль ресурсов. Предоставляет для resources_high.c
 * функцию инициализации всех систем и загрузки всех ресурсов:
 * - systems_init() - инициализация всех систем;
 * - font_load() - загрузка шрифта;
 * - textures_load() - загрузка текстур;
 * - sounds_load() - загрузка звуков;
 * - map_load() - загрузка карты.
 */

#ifndef RESOURCES_MEDIUM_H
#define RESOURCES_MEDIUM_H

void init_system(void);

void load_font(void);

void load_textures(void);

void load_sounds(void);

void load_map(void);

#endif
